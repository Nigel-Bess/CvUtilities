#include <iostream>
#include <Fulfil.CPPUtils/comm/GrpcService.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/commands/dispense_command.h>
#include "commands/tcs/tcs_perception.h"
#include "commands/tcs/tcs_error_codes.h"
#include "commands/tcs/tcs_response.h"

using fulfil::utils::commands::DispenseCommand;

std::shared_ptr<DcResponse> to_response(std::string cmd_id, std::string response){
    auto api_resp = std::make_shared<DcResponse>();
    api_resp->set_command_id(cmd_id);
    Logger::Instance()->Debug("Response id: {}", api_resp->command_id());
    api_resp->set_type(MESSAGE_TYPE_GENERIC_QUERY);
    api_resp->set_message_size(response.size());
    api_resp->set_message_data(response.data(), response.size());
    return api_resp;
}

void handle_request(std::shared_ptr<GrpcService> port, std::shared_ptr<DepthCameras::DcRequest> request, int remainingRetries) {
    auto startTime = std::chrono::steady_clock::now();
    
    std::string payload(request->message_data().data(), request->message_size());
    auto cmd_id = std::make_shared<std::string>(request->command_id());
    auto request_json = std::make_shared<nlohmann::json>(nlohmann::json::parse(payload.c_str()));
    auto type = (*request_json)["Type"].get<DispenseCommand>();
    auto pkid = std::make_shared<std::string>((*request_json)["Primary_Key_ID"].get<std::string>());
    Logger::Instance()->Info("tcs::handle_request new request {} with pkid {} and payload: {}", (int)type, *pkid, payload);
    //switch(type){
        //case DispenseCommand::pre_pickup_clip_actuator:
        //{
            try {
                std::shared_ptr<nlohmann::json> result_json = std::make_shared<nlohmann::json>();
                (*result_json)["Error"] = 0;
                (*result_json)["Error_Description"] = "";
                (*result_json)["Tote_Id"] = 1;
                (*result_json)["Tote_Id"] = 1;
                (*result_json)["Primary_Key_ID"] = "0000000000000000000000001";
                (*result_json)["Facility_Id"] = 0;
                (*result_json)["Clip_Open_States"] = nlohmann::json::parse("[true, true, true, true]");
                auto response = to_response(*cmd_id, result_json->dump());
                Logger::Instance()->Info("Sending response: {}", result_json->dump());
                port->QueueResponse(response);

                auto stopTime = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime).count();
                Logger::Instance()->Info("handle_request type={} id={} took {}ms", type, *pkid, duration);
            } catch (std::exception &e) {
                // Re-run auto exposure just in case if no markers were found
                /*(if (remainingRetries > 0 && TCS_percep.success_code == TCSErrorCodes::NoMarkersDetected) {
                    Logger::Instance()->Warn("No markers found at {}, trying to reset exposure", cam->name_);
                    //cam->RunAutoExposure();
                    return handle_request(port, request, remainingRetries-1);
                }*/
            }
        //}
        //default:
            //Logger::Instance()->Error("tcs::handle_request unhandled command {}", (int)type);
    //}
}

void update_port_status(std::shared_ptr<GrpcService> port, std::string port_name, DepthCameras::DcCameraStatusCodes code) {
    Logger::Instance()->Info("{} Sending status code to FC [{}]", port_name,  DcCameraStatusCodes_Name(code));
    DepthCameras::CameraStatusUpdate msg;
    std::string tostr;
    msg.set_command_id(code == 0 ? "cafebabecafebabecafebabe" : "deadbeefdeadbeefdeadbeef"); 
    msg.set_msg_type(DepthCameras::MESSAGE_TYPE_CAMERA_STATUS);
    msg.set_camera_name(port_name);
    msg.set_camera_serial("fake");
    msg.set_status_code(code);
    msg.SerializeToString(&tostr);
    port->AddStatusUpdate(msg.msg_type(), tostr, msg.command_id());
}

int main()
{
    // Open all the grpc ports / "machines" that I own, at the moment, all of them as the TCS monolith!

    auto clipopener_port = std::make_shared<GrpcService>();
    clipopener_port->Run(9501);
    Logger::Instance()->Info("Listening at clipopener.pioneer.fulfil.ai");

    // TODO: Add all other TCS machine endpoints here

    // TODO: Start Orbbec-manager and Vimba-managers!
    // TODO: stop loop after receiving sigterm!
    std::vector<std::shared_ptr<GrpcService>> ports = { clipopener_port };


    while (true) {
        if(clipopener_port->IsConnected()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        Logger::Instance()->Info("Waiting for clipopener connection");
    }
    Logger::Instance()->Info("clipopener connected to FC!");
    update_port_status(clipopener_port, "ClipOpener", DepthCameras::DcCameraStatusCodes::CAMERA_STATUS_CONNECTED);

    while (true) {
        for (auto const &port : ports) {
            if(!port->IsConnected())continue;
            if(!port->HasNewRequest())continue;
            auto request = port->GetNextRequest();
            std::thread(&handle_request, clipopener_port, request, 1).detach();
        }

        // Throttle queue handling slightly
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}