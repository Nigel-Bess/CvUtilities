#include <memory>

#include <Fulfil.CPPUtils/commands/dc_api_error_codes.h>
#include <Fulfil.CPPUtils/commands/dispense_command.h>
#include <Fulfil.CPPUtils/logging.h>
#include "Fulfil.Dispense/bays/bay_manager.h"
#include "Fulfil.DepthCam/core/depth_sensor.h"
#include <iostream>
#include <unistd.h>

using fulfil::dispense::bays::BayCameraStatusHandler;
using fulfil::utils::Logger;

void listen_for_cameras_connected(std::shared_ptr<fulfil::dispense::DispenseBayData> bay_data) {
    // Add a 5 second sleep so we don't spam weird ephemeral states and give extra "just in case" time,
    // pretty much just always fully connected if cameras are gonna connect.
    // Hopefully this can be removed one day once camera naming lifecycle wonkiness is gone
    sleep(5);

    auto cam_status_handler = new BayCameraStatusHandler(bay_data);
    // Listen for future cam status changes
    if (bay_data->lfb_session != nullptr) {
        bay_data->lfb_session->sensor->add_connected_callback(cam_status_handler);
    }
    if (bay_data->tray_session != nullptr) {
        bay_data->tray_session->sensor->add_connected_callback(cam_status_handler);
    }

    // Artificially Trigger handler immediately in case both cams already connected
    if (bay_data->lfb_session != nullptr) {
        // While session's sensor name is < 3 chars long, loop forever!
        while (bay_data->lfb_session && bay_data->lfb_session->sensor->name_.length() < 3) {
            usleep(100000); // Sleep for 100ms to avoid busy waiting
        }
        cam_status_handler->handle_connection_change(bay_data->lfb_session->sensor->name_, bay_data->lfb_session->sensor->last_status_code);
    }
    if (bay_data->tray_session != nullptr) {
        // While session's sensor name is < 3 chars long, loop forever!
        while (bay_data->tray_session && bay_data->tray_session->sensor->name_.length() < 3) {
            usleep(100000); // Sleep for 100ms to avoid busy waiting
        }
        cam_status_handler->handle_connection_change(bay_data->tray_session->sensor->name_, bay_data->tray_session->sensor->last_status_code);
    }
}

fulfil::dispense::bays::BayManager::BayManager(std::shared_ptr<fulfil::dispense::bays::SensorManager> sensor_manager,
                                               std::shared_ptr<fulfil::dispense::bays::BayRunnerFactory> runner_factory,
                                               std::shared_ptr<fulfil::dispense::bays::BayParser> sensor_parser,
                                               int expected_number_bays, bool both_cameras_required, bool frozen)
{
    this->manager = sensor_manager;
    this->factory = runner_factory;
    this->parser = sensor_parser;
    auto expected_bays = parser->get_bay_ids();
    this->bays = std::make_shared<std::vector<std::shared_ptr<BayRunner>>>(); // TODO: This can be refactored now that we just have one bay
    try
    {
        std::shared_ptr<std::vector<std::shared_ptr<fulfil::depthcam::DepthSession>>> sensors = manager->get_connected_sensors();
        int num_sensors = sensors->size();

        if (num_sensors == 0)
        {
            Logger::Instance()->Error("No Cameras Found;");
            // TODO informative throw
            exit(4); // Exit program is no cameras are found to be connected
        }

        for (auto config_bay_id : parser->get_bay_ids())
        {
            std::vector<std::shared_ptr<fulfil::depthcam::DepthSession>> bay_sensors =
                parser->get_bay(sensors, config_bay_id);
            auto bay_data = std::make_shared<DispenseBayData>(bay_sensors, config_bay_id, both_cameras_required);
            // int sensor_num_1 = parser->get_bay(sensors, 1);
            if (bay_data->can_init()) // if one of the first bay's sensors was found
            {
                Logger::Instance()->Info(
                    "Initializing Bay {} because start condition was met!", bay_data->bay_id);

                // This will check for valid frame data
                int frame_check_iterations = 10;
                auto log_refresh = [&iters = frame_check_iterations](auto session, auto tag)
                {
                    if (session) {
                        Logger::Instance()->Info("Initializing {} cameras frame check with {} iterations ", tag, iters);
                        for (int i = 0; i < iters; i++) {
                            if (i % 2 == 0) { Logger::Instance()->Info("Currently at {} iteration: {}", tag, i);}
                            //session->refresh();
                }} };
                log_refresh(bay_data->lfb_session, "LFB");
                log_refresh(bay_data->tray_session, "Tray");
                std::shared_ptr<BayRunner> runner = factory->create(
                    std::stoi(bay_data->bay_id), bay_data->lfb_session, bay_data->tray_session);
                bays->push_back(runner->get_shared_ptr());
                runner->bind_delegates();

                Logger::Instance()->Debug("Initialized thread for bay {}", bay_data->bay_id);
            }
            else
            {
                Logger::Instance()->Error(
                    "Starting condition not met for bay!; Found {} sensors required by bay {}!",
                    bay_data->num_valid_session(),
                    bay_data->bay_id);
            }
            // Listen for underlying LFB / Tray camera sessions and only signal cams ready
            // after the entire Bay is ready to go.
            std::thread status_thread(listen_for_cameras_connected, bay_data);
            status_thread.detach();
        }
    }
    catch (const std::exception &e)
    {
        Logger::Instance()->Error("Issue building Sensors, Bays or Runners; Vars: error = {}", e.what());
        exit(6);
    }
    if (this->bays->empty())
    {
        Logger::Instance()->Error("Failed to initialize any bays out of the {} requested!", expected_bays.size());
        exit(6);
    }
    this->threads = std::make_shared<std::vector<std::shared_ptr<std::thread>>>(bays->size()); // TODO: reduce number of threads here?
}

BayCameraStatusHandler::BayCameraStatusHandler(std::shared_ptr<DispenseBayData> bay_data) {
    this->bay_data = bay_data;
}

void BayCameraStatusHandler::handle_connection_change(std::string cam_name, DepthCameras::DcCameraStatusCodes current_status)
{
    // If camera doesn't even know it's name yet, just bail
    if (cam_name.length() < 3) {
        Logger::Instance()->Info("CamStatus: cam_name not ready yet");
        return;
    }
    // Mirror all non-connected statuses to FC
    if (current_status != DepthCameras::DcCameraStatusCodes::CAMERA_STATUS_CONNECTED) {
        Logger::Instance()->Info("CamStatus: Mirror {} to {}", current_status, cam_name);
        auto session = bay_data->lfb_session != nullptr && cam_name == bay_data->lfb_session->sensor->name_ ? bay_data->lfb_session : bay_data->tray_session;
        session->sensor->create_camera_status_msg();
        return;
    }
    // Only signal fully connected to FC when both cameras are ready
    if (bay_data->lfb_session != nullptr && bay_data->tray_session != nullptr) {
        if (bay_data->lfb_session->sensor->last_status_code == DepthCameras::DcCameraStatusCodes::CAMERA_STATUS_CONNECTED &&
            bay_data->tray_session->sensor->last_status_code == DepthCameras::DcCameraStatusCodes::CAMERA_STATUS_CONNECTED) {
            Logger::Instance()->Info("CamStatus: Tray and LFB ready {} {}", current_status, cam_name);
            bay_data->tray_session->sensor->create_camera_status_msg();
            bay_data->lfb_session->sensor->create_camera_status_msg();
        }
    }
    // Signal to FC if only 1 cam is required and ready
    else if (bay_data->lfb_session != nullptr && bay_data->lfb_session->sensor->last_status_code == DepthCameras::DcCameraStatusCodes::CAMERA_STATUS_CONNECTED) {
        Logger::Instance()->Info("CamStatus: Mirror LFB connected status for {}", cam_name);
        bay_data->lfb_session->sensor->create_camera_status_msg();
    }
    else if (bay_data->tray_session != nullptr && bay_data->tray_session->sensor->last_status_code == DepthCameras::DcCameraStatusCodes::CAMERA_STATUS_CONNECTED) {
        Logger::Instance()->Info("CamStatus: Mirror Tray connected status for {}", cam_name);
        bay_data->tray_session->sensor->create_camera_status_msg();
    } else {
        Logger::Instance()->Info("CamStatus: Still not ready: lfb_session_null: {} tray_session_null: {} lfb_status: {} tray_status: {}", 
        bay_data->lfb_session == nullptr, 
        bay_data->tray_session == nullptr, 
        bay_data->lfb_session->sensor->last_status_code, 
        bay_data->tray_session->sensor->last_status_code);
    }
}

void fulfil::dispense::bays::BayManager::start() {
    curl_global_init(CURL_GLOBAL_ALL);
    for (int i = 0; i < this->bays->size(); i++)
    {
        if (this->bays->at(i) == nullptr)
            continue; // Do not try to start bays that have not been created
        threads->at(i) = std::make_shared<std::thread>([this, i]()
                                                       { this->bays->at(i)->start(); });
    }

    for (int i = 0; i < this->threads->size(); i++)
    {
        if (this->bays->at(i) == nullptr)
            continue;
        this->threads->at(i)->join();
    }
}
