#include <memory>

#include <Fulfil.CPPUtils/commands/dc_api_error_codes.h>
#include <Fulfil.CPPUtils/commands/dispense_command.h>
#include <Fulfil.CPPUtils/file_system_util.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/networking/socket_network_manager.h>
#include <Fulfil.CPPUtils/timer.h>
#include <Fulfil.DepthCam/data.h>
#include <Fulfil.DepthCam/data/bigquery_upload.h>
#include <Fulfil.DepthCam/mocks.h>
#include <Fulfil.Dispense/commands/code_response.h>
#include <Fulfil.Dispense/commands/content_response.h>
#include <Fulfil.Dispense/commands/drop_target/drop_target_response.h>
#include <Fulfil.Dispense/commands/error_response.h>
#include <Fulfil.Dispense/commands/parsing/dispense_json_parser.h>
#include <Fulfil.Dispense/commands/parsing/dispense_request_parser.h>
#include <Fulfil.Dispense/commands/parsing/tray_parser.h>
#include <Fulfil.Dispense/commands/pre_side_dispense/pre_side_dispense_response.h>
#include "Fulfil.Dispense/dispense/dispense_manager.h"
#include <Fulfil.Dispense/dispense/dispense_processing_queue_predicate.h>
#include <Fulfil.Dispense/drop/side_drop_result.h>
#include <Fulfil.Dispense/tray/tray_algorithm.h>
#include <Fulfil.Dispense/visualization/live_viewer.h>
#include <Fulfil.Dispense/visualization/make_media.h>
#include <iostream>
#include <unistd.h>

using fulfil::depthcam::Session;
using fulfil::depthcam::aruco::Container;
using fulfil::depthcam::aruco::MarkerDetector;
using fulfil::depthcam::aruco::MarkerDetectorContainer;
using fulfil::depthcam::data::BQUpload;
using fulfil::depthcam::data::DataGenerator;
using fulfil::depthcam::data::FileSender;
using fulfil::depthcam::data::GCSSender;
using fulfil::depthcam::data::UploadGenerator;
using fulfil::depthcam::mocks::MockSession;
using fulfil::depthcam::pointcloud::PointCloud;
using fulfil::dispense::DispenseManager;
using fulfil::dispense::commands::DispenseJsonParser;
using fulfil::dispense::commands::DispenseRequest;
using fulfil::dispense::commands::DispenseRequestParser;
using fulfil::dispense::commands::DispenseResponse;
using fulfil::dispense::commands::DropTargetDetails;
using fulfil::dispense::commands::DropTargetDetails;
using fulfil::dispense::commands::ErrorResponse;
using fulfil::dispense::commands::FloorViewResponse;
using fulfil::dispense::commands::ItemEdgeDistanceResponse;
using fulfil::dispense::commands::PostLFRResponse;
using fulfil::dispense::commands::PreSideDispenseResponse;
using fulfil::dispense::commands::TrayValidationResponse;
using fulfil::dispense::commands::TrayViewResponse;
using fulfil::dispense::drop::DropManager;
using fulfil::dispense::drop::DropResult;
using fulfil::dispense::drop::SideDropResult;
using fulfil::dispense::tray::Tray;
using fulfil::dispense::tray_processing::TrayAlgorithm;
using fulfil::dispense::visualization::LiveViewer;
using fulfil::dispense::visualization::ViewerImageType;
using fulfil::utils::commands::dc_api_error_codes::DcApiErrorCode;
using fulfil::utils::commands::dc_api_error_codes::get_error_name_from_code;
using fulfil::utils::commands::DispenseCommand;
using fulfil::utils::FileSystemUtil;
using fulfil::utils::Logger;
using fulfil::utils::ini::IniSectionReader;
using fulfil::utils::networking::SocketInformation;
using fulfil::utils::networking::SocketManager;
using fulfil::utils::networking::SocketNetworkManager;
using fulfil::utils::processingqueue::ProcessingQueue;
using fulfil::utils::processingqueue::ProcessingQueuePredicate;

DispenseManager::DispenseManager(
    int bay, std::shared_ptr<fulfil::depthcam::Session> LFB_session,
    std::shared_ptr<fulfil::depthcam::Session> tray_session,
    std::shared_ptr<INIReader> dispense_man_reader,
    std::shared_ptr<INIReader> tray_config_reader,
    std::shared_ptr<fulfil::dispense::tray::TrayManager> tray_manager) : bay(bay),
                                                                         dispense_reader(dispense_man_reader), tray_config_reader(tray_config_reader)
{
    Logger::Instance()->Trace("DispenseManager Constructor Called");

    // setting up networking stuff
    //  TODO Once needs stabilize, we should probably just add data members in reader to interface
    //  This should be the final place that the reader pointer is passed!
    auto dispense_name = std::string("dispense_") + char(this->bay + 48);
    auto safe_get_dispense_string_val = [this, dispense_name = std::string("dispense_") + char(this->bay + 48)](auto key, std::string default_value)
    {
        return this->dispense_reader->Get(dispense_name, key, default_value);
    };
    this->machine_name = safe_get_dispense_string_val("name", dispense_name);
    this->machine_id = safe_get_dispense_string_val("_id", "000000000000000000000000");
    int port = this->dispense_reader->GetInteger(dispense_name, "port", 9500);

    auto safe_get_tray_string_val =
        [&tray_dim_type = this->tray_dimension_type, &tray_config = this->tray_config_reader](auto key, std::string default_value)
    { return tray_config->Get(tray_dim_type, key, default_value); };
    this->store_id = safe_get_tray_string_val("store_id", "f0");
    this->cloud_media_bucket = safe_get_tray_string_val("cloud_media_bucket", "factory-media");

    Logger::Instance()->Info("Initializing socket network manager at configured port {} for machine {}", port, this->machine_name);

    auto parser = std::make_shared<DispenseRequestParser>(std::make_shared<DispenseJsonParser>());
    socket_manager = std::make_shared<SocketManager>(std::make_shared<SocketInformation>(port));

    this->network_manager = std::make_shared<SocketNetworkManager<std::shared_ptr<DispenseRequest>>>(socket_manager, parser);

    if (tray_session)
    {
        // Sensor is connected
        Logger::Instance()->Trace("Tray session is not null");
        this->tray_session = tray_session;
        std::cout << "Setting service tray" << std::endl;
        this->tray_session->set_service(this->socket_manager->service_);

        // read specific tray configuration
        std::string tray_config_type = this->dispense_reader->Get("device_specific", "tray_config_type");
        this->tray_dimension_type = "tray_dimensions_" + tray_config_type;
        Logger::Instance()->Info("AGX specific ini file indicates to use tray config section {}", this->tray_dimension_type);

        this->tray_manager = tray_manager;
    }

    if (LFB_session)
    {
        // Sensor is connected
        Logger::Instance()->Trace("LFB session is not null");
        this->LFB_session = LFB_session;
        std::cout << "Setting service LFB" << std::endl;
        this->LFB_session->set_service(this->socket_manager->service_);

        if (dispense_man_reader->GetBoolean(dispense_man_reader->get_default_section(), "live_visualize_drop", false))
        {
            std::string live_viewer_path = this->dispense_reader->Get(dispense_man_reader->get_default_section(), "live_visualize_drop_path", "");
            if (live_viewer_path.empty())
            {
                Logger::Instance()->Warn("Live Viewer Drop Output Path not specified. Check main.ini file");
            }
            else
            {
                std::string cloud_directory = make_media::paths::join_as_path(this->store_id, "dispenses").string();
                std::shared_ptr<GCSSender> gcs_sender = std::make_shared<GCSSender>(this->cloud_media_bucket, cloud_directory);
                this->live_viewer = std::make_shared<LiveViewer>(live_viewer_path, gcs_sender);
                Logger::Instance()->Debug("Live Viewer on Drop for bay: {}, set up with base path: {}", bay, live_viewer_path);
            }
        }
    }

    this->drop_manager = std::make_shared<DropManager>(this->LFB_session, dispense_man_reader, this->live_viewer);
    this->processing_queue = std::make_shared<ProcessingQueue<std::shared_ptr<DispenseRequest>, std::shared_ptr<DispenseResponse>>>();

    // separate uploader for keeping track of if we should stop saving the LFB video
    // this->stop_lfb_video_saving = std::make_shared<bool>(true);
    // this->stop_lfb_video_delay = std::make_shared<int>(0);
    this->lfb_upload_generator = std::make_shared<UploadGenerator>(0);

    // separate uploader for keeping track of if we should stop saving the tray video
    // this->stop_tray_video_saving = std::make_shared<bool>(true);
    // this->stop_tray_video_delay = std::make_shared<int>(0);
    this->tray_upload_generator = std::make_shared<UploadGenerator>(0);

    if (this->LFB_session != nullptr)
    {
        this->LFB_session->set_emitter(true); // by default turn the LFB projector emitter to the OFF state
    }

    if (this->tray_session != nullptr)
    {
        this->tray_session->set_emitter(false); // by default turn the tray projector emitter to the OFF state
    }
}

void DispenseManager::bind_delegates()
{
    this->network_manager->delegate = this->weak_from_this();
    this->processing_queue->delegate = this->weak_from_this();
}

void DispenseManager::start()
{
    Logger::Instance()->Info("Starting bay {}: {}", this->bay, this->machine_name);
    this->network_manager->start_listening();
    // this->processing_queue->start_processing(5);
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

void DispenseManager::handle_request(std::shared_ptr<std::string> payload, std::shared_ptr<std::string> command_id)
{
    std::thread(&DispenseManager::handle_request_in_thread, this, payload, command_id).detach();
}

void DispenseManager::handle_request_in_thread(std::shared_ptr<std::string> payload, std::shared_ptr<std::string> command_id)
{
    std::cout << "payload---> " << payload.get()->c_str() << std::endl;
    auto request_json = std::make_shared<nlohmann::json>(nlohmann::json::parse(payload->c_str()));
    auto type = (*request_json)["Type"].get<DispenseCommand>();

    auto pkid = std::make_shared<std::string>((*request_json)["Primary_Key_ID"].get<std::string>());
    std::shared_ptr<DispenseResponse> response;
    switch (type)
    {
    case DispenseCommand::request_bag_state:
    {
        Logger::Instance()->Info("Received Get State Request on Bay {}, PKID: {}, request_id: {}",
                                 this->machine_name, *pkid, *command_id);
        auto result = handle_get_state(pkid, request_json);
        response = std::make_shared<fulfil::dispense::commands::ContentResponse>(
            command_id, std::make_shared<std::string>(result),
            DepthCameras::MessageType::MESSAGE_TYPE_BAG_STATE_REQUEST);
        break;
    }
    case DispenseCommand::send_bag_state:
    {
        Logger::Instance()->Info("Received Update State Request on Bay {}, PKID: {}, request_id: {}",
                                 this->machine_name, *pkid, *command_id);
        auto result = handle_update_state(pkid, request_json);
        response = std::make_shared<fulfil::dispense::commands::CodeResponse>(command_id, result);
        break;
    }
    case DispenseCommand::drop_target:
    {
        Logger::Instance()->Info("Received Drop Target Request on Bay {}, PKID: {}, request_id: {}",
                                 this->machine_name, *pkid, *command_id);
        auto drop_details = std::make_shared<fulfil::dispense::commands::DropTargetDetails>(request_json,
                                                                                            command_id);
        auto raw_result = handle_drop_target(drop_details, request_json);
        if (raw_result->success_code == 0 || raw_result->success_code == 9)
        {
            response = std::make_shared<fulfil::dispense::commands::DropTargetResponse>(command_id,
                                                                                        raw_result->success_code,
                                                                                        raw_result->rover_position,
                                                                                        raw_result->dispense_position,
                                                                                        raw_result->depth_result,
                                                                                        raw_result->max_depth_point_X,
                                                                                        raw_result->max_depth_point_Y,
                                                                                        raw_result->max_Z,
                                                                                        raw_result->Rotate_LFB,
                                                                                        raw_result->LFB_Currently_Rotated,
                                                                                        raw_result->Swing_Collision_Expected,
                                                                                        raw_result->target_depth_range,
                                                                                        raw_result->target_depth_variance,
                                                                                        raw_result->interference_max_z,
                                                                                        raw_result->interference_average_z,
                                                                                        raw_result->target_region_max_z,
                                                                                        raw_result->error_description);
        }
        else
            response = std::make_shared<fulfil::dispense::commands::DropTargetResponse>(command_id,
                                                                                        raw_result->success_code,
                                                                                        raw_result->error_description);
        break;
    }
    case DispenseCommand::pre_LFR:
    {
        Logger::Instance()->Info("Received Pre Drop LFB Request on Bay {}, PKID: {}, request_id: {}",
                                 this->machine_name, *pkid, *command_id);
        auto code = handle_pre_LFR(pkid, request_json);
        response = std::make_shared<fulfil::dispense::commands::CodeResponse>(command_id, code);
        break;
    }
    case DispenseCommand::post_LFR:
    {
        Logger::Instance()->Info("Received Post Drop Request on Bay {}, PKID: {}, request_id: {}",
                                 this->machine_name, *pkid, *command_id);
        response = handle_post_LFR(pkid, command_id, request_json);
        break;
    }
    case DispenseCommand::floor_view: {
        Logger::Instance()->Info("Received Floor View Request on Bay {}, PKID: {}, request_id: {}",
                                 this->machine_name, *pkid, *command_id);
        response = handle_floor_view(pkid, command_id, request_json);
        break;
    }
    case DispenseCommand::tray_view: {
        Logger::Instance()->Info("Received Tray View Request on Bay {}, PKID: {}, request_id: {}",
                                 this->machine_name, *pkid, *command_id);
        response = handle_tray_view(pkid, command_id, request_json);
        break;
    }
    case DispenseCommand::side_dispense_target:
    {
        Logger::Instance()->Info("Received Side Dispense Target Request on Bay {}, PKID: {}, request_id: {}",
                                 this->machine_name, *pkid, *command_id);
        response = handle_side_dispense_target(command_id, request_json);
        break;
    }
    case DispenseCommand::pre_side_dispense:
    {
        Logger::Instance()->Info("Received Pre Side Dispense Request on Bay {}, PKID: {}, request_id: {}",
                                 this->machine_name, *pkid, *command_id);
        response = handle_pre_side_dispense(command_id, request_json);
        break;
    }
    case DispenseCommand::post_side_dispense:
    {
        Logger::Instance()->Info("Received Post Side Dispense Request on Bay {}, PKID: {}, request_id: {}",
                                 this->machine_name, *pkid, *command_id);
        response = handle_post_side_dispense(command_id, request_json);
        break;
    }
    case DispenseCommand::start_lfb_video:
    {
        Logger::Instance()->Info("Received Start LFB Video Request on Bay {}, PKID: {}, request_id: {}",
                                 this->machine_name, *command_id);
        handle_start_lfb_video(pkid);
        response = std::make_shared<fulfil::dispense::commands::CodeResponse>(command_id, 0);
        break;
    }
    case DispenseCommand::stop_lfb_video:
    {
        Logger::Instance()->Info("Received Stop LFB Video Request on Bay {}, PKID: {}, request_id: {}",
                                 this->machine_name, *pkid, *command_id);
        handle_stop_lfb_video();
        response = std::make_shared<fulfil::dispense::commands::CodeResponse>(command_id, 0);
        break;
    }
    case DispenseCommand::start_tray_video:
    {
        Logger::Instance()->Info("Received Start Tray Video Request on Bay {}, PKID: {}, request_id: {}",
                                 this->machine_name, *pkid, *command_id);
        handle_start_tray_video(pkid);
        response = std::make_shared<fulfil::dispense::commands::CodeResponse>(command_id, 0);
        break;
    }
    case DispenseCommand::stop_tray_video:
    {
        Logger::Instance()->Info("Received Stop Tray Video Request on Bay {}, PKID: {}, request_id: {}",
                                 this->machine_name, *pkid, *command_id);
        int delay = 0;
        if (request_json->contains("Delay_Ms"))
        {
            delay = std::min(5000, (*request_json)["Delay_Ms"].get<int>());
            // usleep(1000*delay);
            Logger::Instance()->Debug("Received stop video delay of {} ms", delay);
        }
        handle_stop_tray_video(delay);
        response = std::make_shared<fulfil::dispense::commands::CodeResponse>(command_id, 0);
        break;
    }
    case DispenseCommand::tray_validation:
    {
        Logger::Instance()->Info("Received Tray Validation Request on Bay {}, PKID: {}, request_id: {}",
                                 this->machine_name, *pkid, *command_id);
        response = handle_tray_validation(command_id, request_json);
        break;
    }
    case DispenseCommand::item_edge_distance:
    {
        Logger::Instance()->Info("Received Tray Dispense Lane Request on Bay {}, PKID: {}, request_id: {}",
                                 this->machine_name, *pkid, *command_id);
        response = handle_item_edge_distance(command_id, request_json);
        break;
    }
    default:
    {
        Logger::Instance()->Error("Un-handled request type on Bay {}, PKID: {}, request_id: {}", this->machine_name,
                                  *pkid, *command_id);
        response = std::make_shared<fulfil::dispense::commands::CodeResponse>(command_id, (int)type);
        break;
    }
    }
    network_manager->send_response(response);
}

void DispenseManager::did_receive_request(std::shared_ptr<DispenseRequest> request)
{

    Logger::Instance()->Debug("Dispense Manager: about to push request to process queue {}", *request->request_id);
    this->processing_queue->push(request);
}

void DispenseManager::did_receive_invalid_request(std::shared_ptr<std::string> request_id)
{
    this->network_manager->send_response(std::make_shared<ErrorResponse>(request_id));
}

void DispenseManager::did_receive_invalid_request()
{
    this->network_manager->send_response(std::make_shared<ErrorResponse>());
}

void DispenseManager::send_response(std::shared_ptr<DispenseResponse> response)
{
    this->network_manager->send_response(response);
}

std::shared_ptr<DispenseResponse> DispenseManager::process_request(std::shared_ptr<DispenseRequest> request)
{
    return request->execute();
}

std::shared_ptr<std::string> fulfil::dispense::DispenseManager::create_datagenerator_basedir()
{   
    std::shared_ptr<std::string> base_directory = std::make_shared<std::string>(this->dispense_reader->Get(dispense_reader->get_default_section(), "data_gen_image_base_dir", ""));
    if (base_directory->length() == 0)
    {
        Logger::Instance()->Fatal("Could not find data_gen_image_base_dir value in section {} of main_config.ini!", dispense_reader->get_default_section());
        throw std::runtime_error("Issue getting config settings.");
    }
    if (base_directory->back() == '/')
        base_directory->pop_back();
    base_directory->append("_").append(*FileSystemUtil::create_datetime_string(true));

    return base_directory;
}

bool DispenseManager::check_motor_in_position()
{
    return true;
}

#pragma region bag_cam
std::shared_ptr<FloorViewResponse> DispenseManager::handle_floor_view(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<std::string> command_id, std::shared_ptr<nlohmann::json> request_json)
{

    auto timer = fulfil::utils::timing::Timer("DispenseManager::handle_floor_view for " + this->machine_name + " with PKID " + *PrimaryKeyID);
    Logger::Instance()->Debug("Handling Floor View Processing {} for Bay: {}", *command_id, this->machine_name);

    if (!this->LFB_session) {
        Logger::Instance()->Warn("No LFB Session on Bay {}: Bouncing Floor View!", this->machine_name);
        return std::make_shared<FloorViewResponse>(command_id, 12, "No LFB Session", false, false, 0);
    }

    this->LFB_session->refresh();

    // initial data generation
    std::shared_ptr<std::string> time_stamp = FileSystemUtil::create_datetime_string();
    std_filesystem::path base_directory = make_media::paths::join_as_path(*this->create_datagenerator_basedir(),
                                                                          "Drop_Camera",
                                                                          *PrimaryKeyID,
                                                                          "Floor_View_Image");
    Logger::Instance()->Debug("Base directory is {}", base_directory.string());
    std::shared_ptr<DataGenerator> generator = std::make_shared<DataGenerator>(this->LFB_session,
                                                                               std::make_shared<std::string>(base_directory.string()),
                                                                               request_json);
    generator->save_data(time_stamp);

    // TODO: run floor view analysis and save result. for now use these defaults
    int error_code = 0;
    std::string error_description = "";
    bool anomaly_detected = false;
    bool item_on_ground = false;
    float floor_analysis_confidence_score = 0;
    std::shared_ptr<FloorViewResponse> floor_result = std::make_shared<FloorViewResponse>(command_id,
                                                                                          error_code,
                                                                                          error_description,
                                                                                          anomaly_detected,
                                                                                          item_on_ground,
                                                                                          floor_analysis_confidence_score);

    // generate results data
    std::string floor_view_file = make_media::paths::join_as_path(base_directory, *time_stamp, "floor_view_result");
    std::string error_code_file = make_media::paths::join_as_path(base_directory, *time_stamp, "error_code");
    this->drop_manager->generate_floor_view_result_data(true,
                                                        floor_view_file,
                                                        error_code_file,
                                                        anomaly_detected,
                                                        item_on_ground,
                                                        floor_analysis_confidence_score, error_code);

    Logger::Instance()->Info("Finished handling Floor View command. Result: "
                             "Bay: {} PKID: {} Anomaly Present: {}, Item on Ground: {}, Bots in Image: {}", this->machine_name, *PrimaryKeyID, anomaly_detected, item_on_ground, floor_analysis_confidence_score);
    return floor_result;
}

std::shared_ptr<DropResult> DispenseManager::handle_drop_target(std::shared_ptr<fulfil::dispense::commands::DropTargetDetails> details,
                                                                                        std::shared_ptr<nlohmann::json> request_json)
{
    std::string PrimaryKeyID = (*request_json)["Primary_Key_ID"].get<std::string>();
    auto timer = fulfil::utils::timing::Timer("DispenseManager::handle_drop_target for " + this->machine_name + " request " + PrimaryKeyID);
    Logger::Instance()->Debug("Handling Drop Target Command {} for Bay: {}", PrimaryKeyID, this->machine_name);
    if (!this->LFB_session)
    {
        Logger::Instance()->Warn("No LFB Session: Bouncing Drop Camera Drop Target");
        return std::make_shared<DropResult>(details->request_id, DcApiErrorCode::AlgorithmFail_Bypass,
                                            "No LFB Session: Bouncing Drop Camera Drop Target"); // Todo: move to throw/catch format, log data
    }
    // set cached drop_target fields back to nullptr
    this->drop_manager->cached_drop_target_container = nullptr;
    this->drop_manager->cached_drop_target_request = nullptr;
    this->drop_manager->cached_drop_target = nullptr;
    this->drop_manager->cached_drop_damage_code = nullptr;

    // set cached pre-drop fields back to nullptr, to ensure a pre-drop image is also taken for every drop target provided
    this->drop_manager->cached_pre_container = nullptr;
    this->drop_manager->cached_pre_request = nullptr;

    Logger::Instance()->Debug("Bag ID in drop target request is: {}", details->bag_id);
    if (details->bag_id.compare(this->bag_id) != 0)
    {
        Logger::Instance()->Error("Drop Target - State Bag ID Mismatch; Vars: state_bag_id = {}; Cam: LFB; PKID: {}",
                                  this->bag_id, PrimaryKeyID);
        return std::make_shared<DropResult>(details->request_id,
                                            DcApiErrorCode::BagIdMismatch,
                                            "Bag ID in drop target request is: `" + details->bag_id +
                                                "` while Bag ID in the dispense manager is: `" + this->bag_id + "`"); // Todo: move to throw/catch format, log data
    }

    std::shared_ptr<std::string> base_directory = this->create_datagenerator_basedir();

    std::shared_ptr<std::string> time_stamp_string = FileSystemUtil::create_datetime_string();

    if (!this->dispense_reader->GetBoolean("device_specific", "use_advanced_damage_criteria", false))
    {
        Logger::Instance()->Warn("Not using advanced damage criteria, check configs");
        details->item_damage_code = details->item_material;
    }
    else
    {
        Logger::Instance()->Info("Using advanced damage criteria, damage code for this item: {}", details->item_damage_code);
    }

    /**
     *  Call method in live viewer to rename certain visualizations if they already exist
     *  This would occur during an LFB Pirouette - where the drop target + markers images will already exist from previous request
     */

    // call drop target algorithm
    std::shared_ptr<DropResult>
        drop_result = this->drop_manager->handle_drop_request(request_json, details, base_directory, time_stamp_string,
                                                              true, this->bot_already_rotated_for_current_dispense);

    // if algorithm failed with no drop target, upload available visualizations immediately
    if (drop_result->success_code != DcApiErrorCode::Success and drop_result->success_code != DcApiErrorCode::EmptyBagNotEmpty)
    {
        if (this->live_viewer != nullptr)
        {
            std::shared_ptr<std::vector<std::string>> message = std::make_shared<std::vector<std::string>>();
            std::string error_line = "Target: Error ID " + std::to_string(drop_result->success_code);
            message->push_back(error_line);
            std::string specific_error_message = get_error_name_from_code((DcApiErrorCode)drop_result->success_code);
            message->push_back(specific_error_message);
            std::cout << specific_error_message << std::endl;
            this->live_viewer->update_image(live_viewer->get_blank_visualization(), ViewerImageType::Info, PrimaryKeyID, true, message);

            Logger::Instance()->Debug("Handle Drop Target Failed with code {}", get_error_name_from_code((DcApiErrorCode)drop_result->success_code));
        }
    }
    else // case for valid drop target result without error
    {
        if (drop_result->Rotate_LFB)
        {
            Logger::Instance()->Info("Drop Target Result recommends to rotate LFB; caching to disallow further rotation on this dispense");
            this->bot_already_rotated_for_current_dispense = true;
        }
    }

    Logger::Instance()->Debug("Finished handling Drop Target Request for Bay: {}", this->machine_name);
    return drop_result;
}


int DispenseManager::handle_update_state(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json)
{
    Logger::Instance()->Trace("Handle Get State called in Dispense Manager");
    if (!this->LFB_session)
    {
        Logger::Instance()->Warn("No LFB Session: Bouncing Drop Camera Get State");
        return 12;
    }

    this->bot_already_rotated_for_current_dispense = false; // get state indicates a new bot, reset this flag here
    try
    {

        auto bag = nlohmann::to_string((*request_json)["Bag_State"]);
        std::cout << "CvBagState JSON from FC: " << bag << std::endl;
        fulfil::mongo::CvBagState cvbag = fulfil::mongo::CvBagState((*request_json)["Bag_State"]);
        std::cout << "SET EQUAL TO BAG_STATE, pre parse" << std::endl;
        this->drop_manager->mongo_bag_state->parse_in_values((std::make_shared<fulfil::mongo::CvBagState>(cvbag)));
        std::cout << "post parse" << std::endl;
        this->bag_id = this->drop_manager->mongo_bag_state->raw_mongo_doc->BagId;
        return 0;
    }
    catch (const std::exception &e) // TODO: update error handling here with different parsing / building errors
    {
        Logger::Instance()->Error("Issue handling get state request: \n\t{}", e.what());
        return 1;
    }
    catch (...)
    {
        Logger::Instance()->Error("Unspecified error in handle_get_state");
        return 1;
    }
}

std::string DispenseManager::handle_get_state(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json)
{
    Logger::Instance()->Trace("Handle Update State called in Dispense Manager");
    try
    {
        if (this->drop_manager->mongo_bag_state->raw_mongo_doc == nullptr)
            this->drop_manager->mongo_bag_state->raw_mongo_doc = std::make_shared<fulfil::mongo::CvBagState>();

        return this->drop_manager->mongo_bag_state->GetStateAsString();
    }
    catch (const std::exception &e)
    {
        Logger::Instance()->Error("Issue handling update state request: \n\t{}", e.what());
        return "-1";
    }
    catch (...)
    {
        return "-2";
    }
}

int DispenseManager::handle_pre_LFR(std::shared_ptr<std::string> PrimaryKeyID,
                                    std::shared_ptr<nlohmann::json> request_json)
{
    auto timer = fulfil::utils::timing::Timer("DispenseManager::handle_pre_LFR for " + this->machine_name + " request " + *PrimaryKeyID);
    Logger::Instance()->Debug("Handling Pre Drop Command {} for Bay: {}", *PrimaryKeyID, this->machine_name);

    if (!this->LFB_session)
    {
        Logger::Instance()->Warn("No LFB Session: Bouncing Drop Camera Pre LFR");
        return 12; // TODO CHANGE CODE TO BE USEFUL !!!!
    }

    try
    {
        float remaining_platform = request_json->value("Remaining_Platform", 0.0);
        Logger::Instance()->Debug("Remaining_Platform from Pre_LFR / PreDropImage request: {}", remaining_platform);

        // this->LFB_session->set_emitter(true); //turn on emitter for imaging
        this->LFB_session->refresh();
        // this->LFB_session->set_emitter(false); //turn off emitter after imaging

        Logger::Instance()->Debug("Getting container for caching now");
        std::shared_ptr<MockSession> mock_session_pre = std::make_shared<MockSession>(this->LFB_session);
        std::shared_ptr<MarkerDetectorContainer> container = this->drop_manager->searcher->get_container(this->drop_manager->get_lfb_vision_config(),
                                                                                                         mock_session_pre,
                                                                                                         this->drop_manager->get_lfb_vision_config()->extend_depth_analysis_over_markers);

        this->drop_manager->cached_pre_container = container;  // cache for potential use in prepostcomparison later
        this->drop_manager->cached_pre_request = request_json; // cache for potential use in prepostcomparison later

        /**
         *  Send images to Live Viewer
         */
        if (this->live_viewer != nullptr)
        {
            this->live_viewer->update_image(this->LFB_session->get_color_mat(), ViewerImageType::LFB_Pre_Dispense, *PrimaryKeyID);
        }

        /**
         *  Data generation. Todo: refactor to reduce total code (mostly repeated code in drop_manager)
         */
        std::shared_ptr<std::string> base_directory = this->create_datagenerator_basedir();

        std::shared_ptr<std::string> time_stamp = FileSystemUtil::create_datetime_string();

        FileSystemUtil::join_append(base_directory, "Drop_Camera");
        FileSystemUtil::join_append(base_directory, *PrimaryKeyID);
        FileSystemUtil::join_append(base_directory, "Pre_Drop_Image");

        // Setting up error code file saving
        std::shared_ptr<std::string> error_code_file = std::make_shared<std::string>(*base_directory);
        FileSystemUtil::join_append(error_code_file, *time_stamp);
        FileSystemUtil::join_append(error_code_file, "error_code");

        Logger::Instance()->Trace("Base directory is {}", *base_directory);
        std::shared_ptr<DataGenerator> generator = std::make_shared<DataGenerator>(this->LFB_session,
                                                                                   base_directory,
                                                                                   request_json);
        generator->save_data(time_stamp);

        Logger::Instance()->Trace("Saving error code file with code 0 at path {}", *error_code_file);
        std::ofstream error_file(*error_code_file);
        error_file << "0";

        return 0;
    }
    catch (const std::exception &e)
    {
        Logger::Instance()->Error("Issue handling pre dispense request: \n\t{}", e.what());

        return 1;
    }
    catch (...)
    {
        Logger::Instance()->Error("Unspecified error caught during pre dispense request handling");

        return 1;
    }
}

std::shared_ptr<PostLFRResponse> DispenseManager::handle_post_LFR(std::shared_ptr<std::string> PrimaryKeyID,
                                                                  std::shared_ptr<std::string> request_id,
                                                                  std::shared_ptr<nlohmann::json> request_json)
{
    auto timer = fulfil::utils::timing::Timer("DispenseManager::handle_post_LFR for " + this->machine_name + " request " + *PrimaryKeyID);
    Logger::Instance()->Debug("Handling Post LFR Command {} for Bay: {}", *PrimaryKeyID, this->machine_name);
    auto make_bounce_error = [&request_id]()
    {
        Logger::Instance()->Warn("No LFB Session: Bouncing Drop Camera Post LFR");
        return std::make_shared<PostLFRResponse>(request_id, 12);
    };

    bool should_early_reject_damage_avoidance = request_json->value("Should_Early_Reject_Damage_Avoidance", false);
    // set cached post_drop fields back to nullptr
    this->drop_manager->cached_post_container = nullptr; // reset to nullptr before processing begins, in case encounter errors and prepostcomparison is not possible
    this->drop_manager->cached_post_request = nullptr;
    this->bot_already_rotated_for_current_dispense = false; // reset this flag because handle_post_dispense indicates a dispense just happened

    std::shared_ptr<std::string> base_directory = this->create_datagenerator_basedir();
    // call post-drop algorithm
    std::shared_ptr<PostLFRResponse> response = (this->LFB_session) ? this->drop_manager->handle_post_LFR(
                                                                          request_json,
                                                                          base_directory, request_id, true)
                                                                    : make_bounce_error();
    // pre-post compare (if applicable)
    if (response->get_success_code() != DcApiErrorCode::Success)
    {
        Logger::Instance()->Warn("Post drop analysis failed, will not do pre/post comparison.");
    }
    else
    {
        int comparison_flag = this->dispense_reader->GetInteger("drop_zone_searcher", "pre_post_comparison", 0);
        if (comparison_flag == 1)
        {
            std::pair<int, int> detection_results = this->drop_manager->handle_pre_post_compare(*PrimaryKeyID);
            response->set_items_dispensed(detection_results);

            // if pre/post comparison failed, there may have been request input issues, we do not do product fit check in this case
            if (detection_results.first != -1)
            {
                std::vector<int> products_to_overflow = this->drop_manager->check_products_for_fit_in_bag(request_json, should_early_reject_damage_avoidance);
                Logger::Instance()->Debug("Bag fit check: found {} products of interest that will no longer fit in this bag", products_to_overflow.size());
                response->set_products_to_overflow(products_to_overflow);
            }
        }
    }

    return response;
}

void DispenseManager::handle_start_lfb_video(std::shared_ptr<std::string> PrimaryKeyID)
{
    std::string base_directory = this->dispense_reader->Get(dispense_reader->get_default_section(), "vid_gen_base_buffer_dir");
    if (!this->LFB_session or !this->lfb_upload_generator)
    {
        Logger::Instance()->Warn("Bouncing Bay {} Drop Camera Video; Either missing session or generator", this->machine_name);
        return;
    }

    // check that previous LFB video has already been stopped, as expected by standard sequence of requests
    if (!this->lfb_upload_generator->stop_video_saving) // Move check to video uploader
    {
        Logger::Instance()->Warn("Previous Drop Camera video may still be running! Will send stop signal now and wait 250ms before proceeding");
        this->lfb_upload_generator->stop_video_delay = 0;
        this->lfb_upload_generator->stop_video_saving = true;
        usleep(250000);
    }

    // the stop signal must be turned OFF before a new video generator begins
    this->lfb_upload_generator->stop_video_saving = false;

    std::string cloud_directory = make_media::paths::join_as_path(this->store_id, "dispenses").string();
    std::shared_ptr<GCSSender> sender = std::make_shared<GCSSender>(this->cloud_media_bucket, cloud_directory);

    std::thread lfb_video_recorder_thread([this, PrimaryKeyID, base_directory, sender]() // start new lfb_video_recorder_thread for saving LFB video
                                          {
                           auto remote_video_path = make_media::paths::join_as_path(*PrimaryKeyID, "LFB_Video");
                           auto local_video_path = make_media::paths::join_as_path(base_directory, remote_video_path);
                           Logger::Instance()->Info("Drop Camera Video for {} starting!", *PrimaryKeyID);
                           this->lfb_upload_generator->save_many_frames(this->LFB_session, sender, 60, 5, local_video_path,
                                                                        remote_video_path, false, true);
                           Logger::Instance()->Info("Drop Camera Video for {} Ended!", *PrimaryKeyID); });
    lfb_video_recorder_thread.detach();
}

void DispenseManager::handle_stop_lfb_video()
{
    Logger::Instance()->Debug("Stopping LFB Video Generators with delay: 0 ms");
    this->lfb_upload_generator->stop_video_delay = 0;
    this->lfb_upload_generator->stop_video_saving = true;
    // use a signal type instead of sleep
    // TODO: this value should be based on the fps value for the video generator (e.g. 5fps = 200ms, 300ms > 200ms)
    usleep(400000); // sleep for long enough for stop flag to take effect, even with framerate pause in save_many_frames
    Logger::Instance()->Debug("Stopping LFB video handling is returning now");
}
#pragma endregion bag_cam

#pragma region tray_cam
std::shared_ptr<TrayViewResponse> DispenseManager::handle_tray_view(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<std::string> command_id, std::shared_ptr<nlohmann::json> request_json)
{
    auto timer = fulfil::utils::timing::Timer("DispenseManager::handle_tray_view for " + this->machine_name + " with PKID " + *PrimaryKeyID);
    Logger::Instance()->Debug("Handling Tray View Processing {} for Bay: {}", *command_id, this->machine_name);

    if (!this->tray_session) {
        Logger::Instance()->Warn("No Tray Session on Bay {}: Bouncing Tray View!", this->machine_name);
        return std::make_shared<TrayViewResponse>(command_id, 12, std::make_shared<std::string>("No Tray Session"));
    }
    this->tray_session->refresh();

    // data generation
    std::shared_ptr<std::string> time_stamp = FileSystemUtil::create_datetime_string();
    std_filesystem::path base_directory = make_media::paths::join_as_path(*this->create_datagenerator_basedir(),
                                                                          "Tray_Camera",
                                                                          *PrimaryKeyID,
                                                                          "Tray_View_Image");
    Logger::Instance()->Debug("Base directory is {}", base_directory.string());
    std::shared_ptr<DataGenerator> generator = std::make_shared<DataGenerator>(this->tray_session,
                                                                               std::make_shared<std::string>(base_directory.string()),
                                                                               request_json);
    generator->save_data(time_stamp);

    int error_code = 0;
    std::shared_ptr<std::string> error_description = std::make_shared<std::string>("");
    std::shared_ptr<TrayViewResponse> tray_result = std::make_shared<TrayViewResponse>(command_id, error_code, error_description);

    Logger::Instance()->Info("Finished handling Tray View command. Bay: {} PKID: {}", this->machine_name, *PrimaryKeyID);
    return tray_result;
}

void DispenseManager::handle_start_tray_video(std::shared_ptr<std::string> PrimaryKeyID)
{
    std::string base_directory = this->dispense_reader->Get(dispense_reader->get_default_section(), "vid_gen_base_buffer_dir");
    if (!this->tray_session or !this->tray_upload_generator)
    {
        // TODO - error code
        Logger::Instance()->Warn("Bouncing Bay {} Tray Camera Video; Either missing session or generator", this->machine_name);
        return;
    }
    // check that previous tray video has already stopped, as expected by standard sequence of requests
    if (!this->tray_upload_generator->stop_video_saving) // Move check to video uploader
    {
        Logger::Instance()->Warn("Previous Tray video may still be running! Will send stop signal now and wait 250ms before proceeding");
        this->tray_upload_generator->stop_video_delay = 0;
        this->tray_upload_generator->stop_video_saving = true;
        usleep(250000);
    }

    // the stop signal must be turned OFF before a new video generator begins
    this->tray_upload_generator->stop_video_saving = false;

    std::string cloud_directory = make_media::paths::join_as_path(this->store_id, "dispenses").string();
    std::shared_ptr<GCSSender> sender = std::make_shared<GCSSender>(this->cloud_media_bucket, cloud_directory);

    std::thread tray_video_recorder_thread([this, PrimaryKeyID, base_directory, sender]() // start new thread for saving tray video
                                           {
                            auto remote_video_path = make_media::paths::join_as_path(*PrimaryKeyID, "Tray_Video");
                            auto local_video_path = make_media::paths::join_as_path(base_directory, remote_video_path);
                            Logger::Instance()->Info("Tray Camera Video for {} starting!", *PrimaryKeyID);

                            this->tray_upload_generator->save_many_frames(this->tray_session, sender, 60, 5, local_video_path,
                                                                          remote_video_path, false, true);
                            Logger::Instance()->Info("Tray Camera Video for {} Ended!", *PrimaryKeyID); });
    tray_video_recorder_thread.detach();
}

void DispenseManager::handle_stop_tray_video(int delay)
{
    Logger::Instance()->Debug("Stopping Tray Video Generators with delay: {} ms", delay);
    this->tray_upload_generator->stop_video_delay = delay;
    this->tray_upload_generator->stop_video_saving = true;
    // TODO: this value should be based on the fps value for the video generator (e.g. 5fps = 200ms, 300ms > 200ms)
    usleep(400000); // sleep for 0.4s long enough for stop flag to take effect, even with framerate pause in save_many_frames
    Logger::Instance()->Debug("Stopping Tray video handling is returning now");
}

void DispenseManager::handle_stop_request(std::shared_ptr<std::string> command_id)
{
    std::shared_ptr<ProcessingQueuePredicate<std::shared_ptr<DispenseRequest>>> predicate = std::make_shared<DispenseProcessingQueuePredicate>(command_id);
    this->processing_queue->purge_queue(predicate);
}

// TODO this is unused, make it useful
results_to_vlsg::TrayValidationCounts dispatch_to_count_api(const std::shared_ptr<fulfil::dispense::tray::TrayManager> &tray_manager,
                                                            std::string &saved_images_base_directory, const request_from_vlsg::TrayRequest &tray_req,
                                                            std::vector<tray_count_api_comms::LaneCenterLine> &center_pixels, std::vector<bool> &tongue_detections)
{
    if (center_pixels.empty())
    {
        return results_to_vlsg::TrayValidationCounts{};
    }
    try
    {
        results_to_vlsg::TrayValidationCounts count_response = tray_manager->dispatch_request_to_count_api(tray_req,
                                                                                                           center_pixels, saved_images_base_directory);
        count_response.update_lane_tongue_detections(tongue_detections);
        Logger::Instance()->Trace("Return body from {} count api query:\n\t{}.", tray_req.get_sequence_step(),
                                  nlohmann::json(count_response).dump());
        return count_response;
    }
    catch (const std::exception &e)
    {
        Logger::Instance()->Error("Issue getting count response for {} Request: \n\t{}", tray_req.get_sequence_step(), e.what());
        return results_to_vlsg::TrayValidationCounts{};
    }
}

// template<typename SaveFN, typename SendFN, typename RunFN>
std::shared_ptr<ItemEdgeDistanceResponse>
DispenseManager::handle_item_edge_distance(std::shared_ptr<std::string> command_id, std::shared_ptr<nlohmann::json> request_json)
{
    // serialize the request
    request_from_vlsg::TrayRequest single_lane_val_req = request_json->get<request_from_vlsg::TrayRequest>();
    auto timer = fulfil::utils::timing::Timer("DispenseManager::handle_item_edge_distance for " + this->machine_name + " request " + single_lane_val_req.m_context.get_id_tagged_sequence_step());
    Logger::Instance()->Debug("Handling {} Dispense Lane Processing {} for Bay: {}", single_lane_val_req.get_sequence_step(), single_lane_val_req.get_primary_key_id(), this->machine_name);

    if (!this->tray_session)
    {
        // TODO fault on this error code in FC
        Logger::Instance()->Warn("No Tray Session on Bay {}: Bouncing Tray Item Edge Distance!", this->machine_name);
        return std::make_shared<ItemEdgeDistanceResponse>(command_id, 12);
    }
    // only refresh once at the beginning of the request handling
    this->tray_session->refresh();

    bool is_pre_dispense = single_lane_val_req.get_sequence_step().substr(0,3) == "Pre";
    Logger::Instance()->Debug("{} Is Pre Dispense: {}", single_lane_val_req.get_sequence_step(), is_pre_dispense); 
    ViewerImageType image_code = is_pre_dispense ? ViewerImageType::Tray_Pre_Dispense : ViewerImageType::Tray_Post_Dispense;
    if (this->live_viewer)
    {
        this->live_viewer->update_image(std::make_shared<cv::Mat>(this->tray_session->grab_color_frame()), image_code, single_lane_val_req.get_primary_key_id(), true);
    }
    std::string saved_images_base_directory = this->dispense_reader->Get(this->dispense_reader->get_default_section(), "data_gen_image_base_dir");
    IniSectionReader section_reader{*this->tray_config_reader, this->tray_dimension_type};
    Tray tray = this->tray_manager->create_tray(single_lane_val_req.m_tray_recipe);

    /** Save Data from generator */
    DataGenerator single_lane_tray_data_generator = this->tray_manager->build_tray_data_generator(
        request_json,
        this->tray_manager->make_default_datagen_path(saved_images_base_directory, 
        single_lane_val_req) / single_lane_val_req.
        get_sequence_step());
    single_lane_tray_data_generator.save_data(std::make_shared<std::string>());

    // run first edge distance processing
    // results_to_vlsg::LaneItemDistance fed_result{};
    // std::vector<tray_count_api_comms::LaneCenterLine> transformed_lane_center_pixels{};
    // std::vector<bool> tongue_detections{};
    TrayAlgorithm tray_algorithm = TrayAlgorithm(section_reader);
    // TODO remove tray session from this func
    auto [fed_result, transformed_lane_center_pixels, tongue_detections] = tray_algorithm.run_tray_algorithm(this->tray_session, single_lane_val_req, tray);
    Logger::Instance()->Debug("Tray algorithm run_tray_algorithm returned FED: {}, now updating image", fed_result.m_first_item_distance);
    // push visualizations for Pre/Post FED to GCP
    // TODO reduce number of these try catches 
    try 
    {
        ViewerImageType image_code = is_pre_dispense ? ViewerImageType::Tray_Pre_FED : ViewerImageType::Tray_Post_FED;
        if (this->live_viewer)
            this->live_viewer->update_image(std::make_shared<cv::Mat>(tray_algorithm.get_FED_visualization_image()), image_code, single_lane_val_req.get_primary_key_id(), true);
        Logger::Instance()->Debug("Tray algorithm updating image succeeded, FED: {}", fed_result.m_first_item_distance);
    } catch (const std::exception &e)
    {
        // TODO add error code here
        Logger::Instance()->Error("Tray algorithm update_image threw exception: {}", e.what());
    }
    
    // do all tray processing
    Logger::Instance()->Debug("Tray algorithm dispatching request ot TC API, FED: {}", fed_result.m_first_item_distance);
    results_to_vlsg::TrayValidationCounts count_response = this->tray_manager->dispatch_request_to_count_api(single_lane_val_req, 
                                                                                 transformed_lane_center_pixels, 
                                                                                 saved_images_base_directory);
    Logger::Instance()->Trace("FED is still: {}", fed_result.m_first_item_distance);
    std::shared_ptr<ItemEdgeDistanceResponse> tray_result = std::make_shared<ItemEdgeDistanceResponse>(fed_result, count_response, command_id);

    Logger::Instance()->Trace("Tray result FED is {}, FED result is {}", tray_result->get_fed_value(), fed_result.m_first_item_distance);
    Logger::Instance()->Info("Finished handling Single Lane Dispense command. Result on Bay: {} PKID: {} First Edge Distance: {}",
                             this->machine_name, single_lane_val_req.get_primary_key_id(), tray_result->get_fed_value());
    return tray_result;
}

// Todo: Should also make a handler class
std::shared_ptr<TrayValidationResponse>
DispenseManager::handle_tray_validation(std::shared_ptr<std::string> command_id, std::shared_ptr<nlohmann::json> request_json)
{
    // todo make trayrequest a TrayValidationRequest
    request_from_vlsg::TrayRequest tray_validation_request = request_json->get<request_from_vlsg::TrayRequest>();
    auto timer = fulfil::utils::timing::Timer("DispenseManager::handle_tray_validation for " + this->machine_name + " request " + tray_validation_request.get_primary_key_id());
    Logger::Instance()->Debug("Handling Tray Validation Command {} for Bay: {}", tray_validation_request.get_primary_key_id(), this->machine_name);

    if (!this->tray_session)
    {
        Logger::Instance()->Warn("No Tray Session: Bouncing Tray Validation Request");
        auto tray_result = nlohmann::json(results_to_vlsg::TrayValidationCounts{});
        tray_result["Error"] = 12;
        return std::make_shared<TrayValidationResponse>(command_id, std::make_shared<std::string>(tray_result.dump()));
    }

    this->tray_session->refresh();

    std::string saved_images_base_directory = this->dispense_reader->Get(this->dispense_reader->get_default_section(), "data_gen_image_base_dir");
    double resize_factor = 1;
    /** Send Data Function */
    std::string local_base_path = this->dispense_reader->Get(this->dispense_reader->get_default_section(), "upload_file_dir", "");

    /** Save Data from generator */
    DataGenerator tray_validation_data_generator = tray_manager->build_tray_data_generator(
        request_json, tray_manager->make_default_datagen_path(saved_images_base_directory, tray_validation_request) / tray_validation_request.get_sequence_step());

    IniSectionReader section_reader{*this->tray_config_reader, this->tray_dimension_type};
    std::shared_ptr<TrayAlgorithm> tray_algorithm = std::make_shared<TrayAlgorithm>(section_reader);
    Tray tray = this->tray_manager->create_tray(tray_validation_request.m_tray_recipe);

    // tray parameters for image cropping
    float tray_width = section_reader.at<float>("tray_width"); 
    float tray_length = section_reader.at<float>("tray_length");

    // rotate image as needed based on the config
    std::shared_ptr<cv::RotateFlags> rotate_code(nullptr);
    int image_rotation_angle_from_camera_placement = this->dispense_reader->GetInteger("device_specific", "image_rotation_angle_from_camera_placement", 0);
    switch (image_rotation_angle_from_camera_placement) {
        case 90:
            rotate_code = std::make_shared<cv::RotateFlags>(cv::ROTATE_90_CLOCKWISE); // 0
            break;
        case 180:
            rotate_code = std::make_shared<cv::RotateFlags>(cv::ROTATE_180); // 1
            break;
        case -90:
            rotate_code = std::make_shared<cv::RotateFlags>(cv::ROTATE_90_COUNTERCLOCKWISE); // 2
            break;
        default:
            rotate_code = nullptr;
            break;
    }
    Logger::Instance()->Debug("Dispense manager image_rotation_angle_from_camera_placement config is {}, and rotate_code RotateFlag is: {}", 
        std::to_string(image_rotation_angle_from_camera_placement), !rotate_code ? "NULL" : std::to_string(*rotate_code));

    // upload tray validation data
    try 
    {
        //save_tray_audit_image for historical debugging and to be used by tray count api
        if (this->tray_manager->save_tray_audit_image(tray_validation_request.m_context, local_base_path, resize_factor, rotate_code, tray_width, tray_length, this->get_induction_cam_distance_from_tray()))
        {
            this->tray_manager->upload_tray_data(tray_validation_request.m_context, local_base_path, GCSSender(this->cloud_media_bucket, this->store_id));
        }
    } catch (const std::exception &e)
    {
        Logger::Instance()->Error("Issue uploading Tray Validation audit data for {}, with exception: {}", tray_validation_request.get_primary_key_id(), e.what());
    }

    results_to_vlsg::TrayValidationCounts count_response;
    try
    {
        tray_validation_data_generator.save_data(std::make_shared<std::string>());
        auto [transformed_lane_center_pixels, tongue_detections, max_height_detected_in_tray] =
            tray_algorithm->get_pixel_lane_centers_and_tongue_detections(
                this->tray_session, tray_validation_request, tray, rotate_code);
        
        count_response = this->tray_manager->dispatch_request_to_count_api(tray_validation_request, transformed_lane_center_pixels, saved_images_base_directory);
        count_response.update_lane_tongue_detections(tongue_detections);
        auto exp_max = tray_validation_request.expected_max_height();
        count_response.m_height_info = results_to_vlsg::TrayHeight(max_height_detected_in_tray, exp_max);
        float detected_expected_height_diff = max_height_detected_in_tray - exp_max;
        float error_over_exp = max_height_detected_in_tray / exp_max;
        Logger::Instance()->Info("Request {} has tallest item tray buffer of {}, and returned height of tray is {} with the confidence {}."
                                    " After adjusting for tray inset, error is {} and detected height is {:0.4f} of expected height.",
                                    tray_validation_request.get_primary_key_id(), exp_max, max_height_detected_in_tray, count_response.m_height_info.m_confidence,
                                    detected_expected_height_diff, error_over_exp);
        Logger::Instance()->Info("Return body from Tray Validation count api query:\n\t{}.", nlohmann::json(count_response).dump());
        // push visualizations for tray validation to GCP
        if (this->live_viewer)
        {
            request_from_vlsg::TrayRequest single_lane_val_req = request_json->get<request_from_vlsg::TrayRequest>();
            this->live_viewer->update_image(std::make_shared<cv::Mat>(tray_algorithm->get_TV_visualization_image()), ViewerImageType::Tray_Validation, single_lane_val_req.get_primary_key_id(), true);
        }
    }
    catch (const std::exception &e)
    {
        // TODO error code
        Logger::Instance()->Error("Issue saving or uploading data from Tray Validation Request: \n\t{}", e.what());
        Logger::Instance()->Info("Sending nominal response in Tray Validation Request");
        count_response = results_to_vlsg::TrayValidationCounts{};
    }

    nlohmann::json tray_result = nlohmann::json(count_response);

    std::shared_ptr<std::string> payload = std::make_shared<std::string>(tray_result.dump());
    Logger::Instance()->Debug("Finished handling Tray Validation Command for Bay: {} PKID: {} Json response:\n\t{}",
                              this->machine_name, tray_validation_request.get_primary_key_id(), *payload);
    return std::make_shared<TrayValidationResponse>(command_id, payload);
}
#pragma endregion tray_cam

std::shared_ptr<fulfil::dispense::bays::BayRunner> fulfil::dispense::DispenseManager::get_shared_ptr()
{
    return this->shared_from_this();
}

#pragma region side_bag
// ***** ALL SIDE DISPENSE-SPECIFIC FUNCTIONALITY FOUND BELOW *****
std::shared_ptr<fulfil::dispense::commands::SideDispenseTargetResponse>
fulfil::dispense::DispenseManager::handle_side_dispense_target(std::shared_ptr<std::string> request_id,
                                                               std::shared_ptr<nlohmann::json> request_json)
{
    auto data_fs_path = make_media::paths::add_basedir_date_suffix_and_join(
                            this->dispense_reader->Get(this->dispense_reader->get_default_section(), "data_gen_image_base_dir"),
                            "Side_Bag_Camera/") /
                        (*request_json)["Primary_Key_ID"].get<std::string>();
    data_fs_path /= "Side_Dispense_Target";
    this->LFB_session->refresh();
    auto data_generator = DataGenerator(this->LFB_session, std::make_unique<std::string>(data_fs_path.string()), request_json);
    data_generator.save_data(std::make_shared<std::string>());
    return std::make_shared<fulfil::dispense::commands::SideDispenseTargetResponse>(request_id);
}

std::shared_ptr<fulfil::dispense::commands::PreSideDispenseResponse>
fulfil::dispense::DispenseManager::handle_pre_side_dispense(std::shared_ptr<std::string> request_id,
                                                            std::shared_ptr<nlohmann::json> request_json)
{
    auto pkid = (*request_json)["Primary_Key_ID"].get<std::string>();
    auto primary_key_id = std::make_shared<std::string>(pkid);
    auto timer = fulfil::utils::timing::Timer("DispenseManager::handle_pre_side_dispense for " + this->machine_name + " request " + pkid);
    Logger::Instance()->Debug("Handling PreSideDispense Command for Bay: {} Request: {}", this->machine_name, pkid);

    if (!this->LFB_session) {
        Logger::Instance()->Warn("No LFB Session. Check all cameras registering and serial numbers match!");
        return std::make_shared<fulfil::dispense::commands::PreSideDispenseResponse>(request_id,
                                                         primary_key_id,
                                                         nullptr,
                                                         -1, -1,
                                                         DcApiErrorCode::UnrecoverableRealSenseError,
                                                         std::string("No LFB Session. Check all cameras registering and serial numbers match!"));
    }
    Logger::Instance()->Trace("Refresh Session Called in Drop Manager -> Handle PreSideDrop");
    // need to refresh the session to get updated frames
    this->LFB_session->refresh();
    this->drop_manager->cached_pre_container = nullptr;  // cache for potential use in prepostcomparison later
    this->drop_manager->cached_pre_request = request_json; // cache for potential use in prepostcomparison later

    // create file path variables for data generation
    std::shared_ptr<std::string> base_directory = this->create_datagenerator_basedir();
    std::shared_ptr<std::string> time_stamp_string = FileSystemUtil::create_datetime_string();

    auto path = make_media::paths::join_as_path(*base_directory, "Side_Bag_Camera", pkid, "Pre_Side_Dispense");
    Logger::Instance()->Debug("Saving PreSideDispense data at path: {}", path.string());
    auto data_generator = DataGenerator(this->LFB_session, std::make_unique<std::string>(path), request_json);
    data_generator.save_data(std::make_shared<std::string>());

    std::shared_ptr<SideDropResult> side_drop_result = this->drop_manager->handle_pre_side_dispense_request(request_id, primary_key_id, request_json, base_directory, time_stamp_string, true);

    std::shared_ptr<fulfil::dispense::commands::PreSideDispenseResponse> pre_side_dispense_response =
        std::make_shared<fulfil::dispense::commands::PreSideDispenseResponse>(request_id, primary_key_id, side_drop_result->occupancy_map, side_drop_result->square_width, side_drop_result->square_height, DcApiErrorCode::Success);

    // if algorithm failed, upload available visualizations immediately
    if (pre_side_dispense_response->success_code != DcApiErrorCode::Success)
    {
        if (this->live_viewer != nullptr)
        {
            std::shared_ptr<std::vector<std::string>> message = std::make_shared<std::vector<std::string>>();
            std::string error_line = "Target: Error ID " + std::to_string(pre_side_dispense_response->success_code);
            message->push_back(error_line);
            std::string specific_error_message = get_error_name_from_code((DcApiErrorCode)pre_side_dispense_response->success_code);
            message->push_back(specific_error_message);
            std::cout << specific_error_message << std::endl;
            this->live_viewer->update_image(live_viewer->get_blank_visualization(), ViewerImageType::Info, *primary_key_id, true, message);

            Logger::Instance()->Debug("Handle PreSideDispense Failed!");
        }
    }

    Logger::Instance()->Debug("Finished handling PreSideDispenseRequest {} for Bay: {}", *primary_key_id, this->machine_name);
    return pre_side_dispense_response;
}

std::shared_ptr<fulfil::dispense::commands::PostSideDispenseResponse>
fulfil::dispense::DispenseManager::handle_post_side_dispense(std::shared_ptr<std::string> request_id,
                                                             std::shared_ptr<nlohmann::json> request_json)
{
    auto pkid = (*request_json)["Primary_Key_ID"].get<std::string>();
    auto primary_key_id = std::make_shared<std::string>(pkid);
    auto timer = fulfil::utils::timing::Timer("DispenseManager::handle_post_side_dispense for " + this->machine_name + " request " + pkid);
    Logger::Instance()->Debug("Handling PostSideDispense Command for Bay: {} Request ID: {}", this->machine_name, pkid);

    // set cached post_drop fields back to nullptr
    this->drop_manager->cached_post_container = nullptr; // reset to nullptr before processing begins, in case encounter errors and prepostcomparison is not possible
    this->drop_manager->cached_post_request = nullptr;

    if (!this->LFB_session) {
        Logger::Instance()->Fatal("No LFB Session. Check all cameras registering and serial numbers match!");
        return std::make_shared<fulfil::dispense::commands::PostSideDispenseResponse>(request_id,
                                                         primary_key_id,
                                                         nullptr,
                                                         -1, -1,
                                                         DcApiErrorCode::UnrecoverableRealSenseError,
                                                         std::string("No LFB Session. Check all cameras registering and serial numbers match!"), 1);
    }
    this->LFB_session->refresh();

    auto data_fs_path = make_media::paths::add_basedir_date_suffix_and_join(
                            this->dispense_reader->Get(this->dispense_reader->get_default_section(), "data_gen_image_base_dir"),
                            "Side_Bag_Camera/") /
                            pkid;
    data_fs_path /= "Post_Side_Dispense";
    auto data_generator = DataGenerator(this->LFB_session, std::make_unique<std::string>(data_fs_path.string()), request_json);
    data_generator.save_data(std::make_shared<std::string>());
    std::shared_ptr<std::string> base_directory = this->create_datagenerator_basedir();


    std::shared_ptr<std::string> time_stamp_string = FileSystemUtil::create_datetime_string();
    std::shared_ptr<SideDropResult> side_drop_result = this->drop_manager->handle_post_side_dispense_request(
            request_id, 
            primary_key_id,
            request_json,
            base_directory, 
            time_stamp_string, 
            true);

    std::shared_ptr<fulfil::dispense::commands::PostSideDispenseResponse> post_side_dispense_response =
        std::make_shared<fulfil::dispense::commands::PostSideDispenseResponse>(request_id, 
            primary_key_id, 
            side_drop_result->occupancy_map, 
            side_drop_result->square_width, 
            side_drop_result->square_height, 
            DcApiErrorCode::Success, 
            "", 1);

    // if algorithm failed, upload available visualizations immediately
    if (post_side_dispense_response->success_code != DcApiErrorCode::Success)
    {
        if (this->live_viewer != nullptr)
        {
            std::shared_ptr<std::vector<std::string>> message = std::make_shared<std::vector<std::string>>();
            std::string error_line = "Target: Error ID " + std::to_string(post_side_dispense_response->success_code);
            message->push_back(error_line);
            std::string specific_error_message = get_error_name_from_code((DcApiErrorCode)post_side_dispense_response->success_code);
            message->push_back(specific_error_message);
            Logger::Instance()->Error("Handle PostSideDispense Failed: {}!", specific_error_message);
            this->live_viewer->update_image(live_viewer->get_blank_visualization(), ViewerImageType::Info, *primary_key_id, true, message);
        }
    }
    else
    {
       int detection_results = this->drop_manager->handle_pre_post_compare_side_dispense(*primary_key_id);
       post_side_dispense_response->items_dispensed = detection_results;
    }
    return post_side_dispense_response;
}
#pragma endregion side_bag

int fulfil::dispense::DispenseManager::get_induction_cam_distance_from_tray() { return this->induction_cam_distance_from_tray; }