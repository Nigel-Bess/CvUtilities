#include "commands/tcs/tcs_error_codes.h"
#include <tuple>
#include <json.hpp>
#include <commands/tcs/tcs_controller.h>
#include <Fulfil.CPPUtils/commands/dispense_command.h>

using fulfil::utils::Logger;
using fulfil::dispense::commands::tcs::BagClipsInference;
using fulfil::dispense::commands::tcs::TCSPerception;
using fulfil::dispense::commands::tcs::TCSController;
using fulfil::dispense::commands::tcs::GrpcPort;
using fulfil::utils::commands::DispenseCommand;

static int MAX_POLL_SLEEP_MS = 10;

TCSController::TCSController(TCSPerception* tcs_perception, OrbbecManager* orbbec_manager) {
    is_listening = false;
    this->tcs_perception = tcs_perception;
    this->orbbec_manager = orbbec_manager;
}

void TCSController::update_port_status(GrpcPort *port, DepthCameras::DcCameraStatusCodes code) {
    Logger::Instance()->Info("{} Sending status code to FC [{}]", port->name,  DcCameraStatusCodes_Name(code));
    DepthCameras::CameraStatusUpdate msg;
    std::string tostr;
    msg.set_command_id(code == 0 ? "cafebabecafebabecafebabe" : "deadbeefdeadbeefdeadbeef"); 
    msg.set_msg_type(DepthCameras::MESSAGE_TYPE_CAMERA_STATUS);
    msg.set_camera_name(port->name);
    msg.set_camera_serial("fake");
    msg.set_status_code(code);
    msg.SerializeToString(&tostr);
    port->service->AddStatusUpdate(msg.msg_type(), tostr, msg.command_id());
}

std::shared_ptr<DcResponse> TCSController::handle_pre_pick_clip_actuator_request(GrpcPort *port, std::string &pkid, std::string &cmd_id) {
    // refresh cam image
    // TODO remove all lfr version stuff
    auto start_time = std::chrono::steady_clock::now();
    
    std::string lfr_version = "LFB-3.2";
    std::string directory_path = "idk";
    std::shared_ptr<cv::Mat> bag_image = nullptr;
    
    std::shared_ptr<nlohmann::json> result_json = std::make_shared<nlohmann::json>();
    (*result_json)["Error"] = 0;
    (*result_json)["Error_Description"] = "";
    (*result_json)["Tote_Id"] = 1;
    (*result_json)["Primary_Key_ID"] = pkid;
    (*result_json)["Facility_Id"] = 0;
    (*result_json)["Clip_Open_States"] = nlohmann::json::parse("[true, true, true, true]");
    auto json_str = result_json->dump();
    auto response = to_response(cmd_id, json_str);
    Logger::Instance()->Info("Sending response: {}", result_json->dump());
    port->service->QueueResponse(response);

    auto stopTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - start_time).count();
    Logger::Instance()->Info("handle_pre_pick_clip_actuator_request: id={} took {}ms", pkid, duration);
    return response;

    //auto clip_states = tcs_perception->getBagClipStates(bag_image, lfr_version, pkid, directory_path);
    //return clip_states;
}

std::shared_ptr<DcResponse> TCSController::to_response(std::string &cmd_id, std::string &response){
    auto api_resp = std::make_shared<DcResponse>();
    api_resp->set_command_id(cmd_id);
    Logger::Instance()->Debug("Response id: {}", api_resp->command_id());
    api_resp->set_type(MESSAGE_TYPE_GENERIC_QUERY);
    api_resp->set_message_size(response.size());
    api_resp->set_message_data(response.data(), response.size());
    return api_resp;
}

void TCSController::start_listening() {
    is_listening = true;
    std::thread(&TCSController::listen_loop, this).detach();
}

void TCSController::stop_listening() {
    is_listening = false;
    // Sleep for long enough to ensure all listen loops have stopped, a little hacky but works at the cost
    // of marginally slower graceful shutdowns
    std::this_thread::sleep_for(std::chrono::milliseconds(MAX_POLL_SLEEP_MS * 2));
}

void TCSController::handle_request(GrpcPort *port, std::shared_ptr<DepthCameras::DcRequest> request, int remainingRetries) {
    auto start_time = std::chrono::steady_clock::now();
    
    std::string payload(request->message_data().data(), request->message_size());
    auto cmd_id = request->command_id();
    auto request_json = std::make_shared<nlohmann::json>(nlohmann::json::parse(payload.c_str()));
    auto type = (*request_json)["Type"].get<DispenseCommand>();
    auto pkid = (*request_json)["Primary_Key_ID"].get<std::string>();
    Logger::Instance()->Info("tcs::handle_request new request {} with pkid {} and payload: {}", (int)type, pkid, payload);

    try {
        switch(type){
            case DispenseCommand::pre_pickup_clip_actuator:
            {
                handle_pre_pick_clip_actuator_request(port, pkid, cmd_id);
                return;
            }
            default:
                Logger::Instance()->Error("tcs::handle_request unhandled command {}", (int)type);
        }
    } catch (std::exception &e) {
        // Re-run auto exposure just in case if no markers were found
        //(if (remainingRetries > 0 && TCS_percep.success_code == TCSErrorCodes::NoMarkersDetected) {
        //    Logger::Instance()->Warn("No markers found at {}, trying to reset exposure", cam->name_);
        //    //cam->RunAutoExposure();
        //    return handle_request(port, request, remainingRetries-1);
        //}
    }
}

GrpcPort* TCSController::open_port(std::string cam_name) {
    auto service = new GrpcService();
    auto port = new GrpcPort(cam_name, service);
    service->Run(9501);
    Logger::Instance()->Info("Listening at {}.pioneer.fulfil.ai:9501", cam_name);

    // TODO: use observer pattern here
    update_port_status(port, DepthCameras::DcCameraStatusCodes::CAMERA_STATUS_CONNECTED); 
    return port;
}

void TCSController::listen_loop() {

    // Setup ports
    auto clipopener_port = open_port("clipopener");

    // TODO: Add all other TCS machine endpoints here
    std::vector<GrpcPort*> ports = { clipopener_port };

    Logger::Instance()->Info("clipopener connected to FC!");

    while (is_listening) {
        bool request_seen = false;
        for (auto const &port : ports) {
            if(!port->service->IsConnected()) continue;
            if(!port->service->HasNewRequest()) continue;
            request_seen = true;
            auto request = port->service->GetNextRequest();
            std::thread(&TCSController::handle_request, this, port, request, 1).detach();
        }

        // Throttle queue handling slightly upon no-ops
        if (!request_seen) {
            std::this_thread::sleep_for(std::chrono::milliseconds(MAX_POLL_SLEEP_MS));
        }
    }

    // TODO: Only get here after stop_listening() is called, so cleanup by emitting disconnects
    for (auto const &port : ports) {
        //port.service->
    }

}
