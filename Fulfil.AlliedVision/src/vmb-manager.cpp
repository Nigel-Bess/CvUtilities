#include "vmb-manager.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <tuple>
#include "commands/bag_release/repack_perception.h"

using namespace fulfil::dispense::commands;

struct sigaction old_action;
bool RUN = true;

void sigint_handler(int sig_no){
    printf("CTRL-C pressed\n");
    RUN = false;
}

VmbManager::VmbManager(std::map<int, std::string> cam_map, fulfil::utils::Logger* logger){
    log_ = logger;
    service_ = std::shared_ptr<GrpcService>(new GrpcService());
    for(auto const & [bay, ip] : cam_map){
        log_->Info("Setting up new VmbCamera on Repack Bay {} with IP:{}",bay ,ip);
        cameras_.emplace(bay, std::make_shared<VmbCamera>(ip, bay, log_, service_));
    }
    RunManager();//block
    std::thread(&VmbManager::RunManager, this).detach();
}

void VmbManager::RunManager(){
    if(!RUN)return;
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = &sigint_handler;
    sigaction(SIGINT, &action, &old_action);

    log_->Info("VmbManager starting");
    auto code = vmb_system.Startup();
    if(code != VmbErrorSuccess){
        log_->Error("Error starting up VmbSystem, failed with code {}!", GetVimbaCode(code));
        return;
    }
    // VmbCPP::FeaturePtrVector feat;
    // code = vmb_system.GetFeatures(feat);
    // std::cout << GetVimbaCode(code) << std::endl;
    // for(auto const & f : feat){
    //     std::string name;
    //     f->GetDisplayName(name);
    //     std::cout << name << std::endl;

    // }
    log_->Info("VmbManager startup successful");
    auto found = 0;
    // CameraPtrVector cameras;
    // code = vmb_system.GetCameras(cameras);
    for(auto const & [ip, cam] : cameras_){
        if(!RUN)break;
        for(auto t = 0; t < 5; t++){
            if(!RUN)break;
            auto code = vmb_system.GetCameraByID(cam->camera_ip_.c_str(), cam->camera_);
            if(code != VmbErrorSuccess){
                log_->Error("Could not get camera {} at {} [{}]", cam->name_, cam->camera_ip_, GetVimbaCode(code));
                cam->KillCamera();
            }
            else {
                cam->StartCamera();
                found++;
                break;
            }
        }
    }
    if(!found){
        log_->Error("No cameras found, exiting");
        RUN = false;
    }
    service_->Run(9395);
    log_->Info("{} Cameras initialized", found);
    while (RUN){
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if(!service_->IsConnected())continue;
        if(!service_->HasNewRequest())continue;
        auto request = service_->GetNextRequest();
        std::thread(&VmbManager::HandleRequest, this, request).detach();
        // for(auto const & [ip, cam] : cameras_)cam->GetImageBlocking();
    }
    for(auto const & [ip, cam] : cameras_){
        cam->KillCamera();
    }
    vmb_system.Shutdown();
    log_->Info("Exiting VmbManager after shutdown");
}

void VmbManager::SendResponse(std::string cmd_id, std::string response){
    auto api_resp = std::make_shared<DcResponse>();
    api_resp->set_command_id(cmd_id);
    log_->Debug("Response id: {}", api_resp->command_id());
    api_resp->set_type(MESSAGE_TYPE_GENERIC_QUERY);
    api_resp->set_message_size(response.size());
    api_resp->set_message_data(response.data(), response.size());
    service_->QueueResponse(api_resp);
}

void VmbManager::HandleRequest(std::shared_ptr<DepthCameras::DcRequest> request){
    std::string payload(request->message_data().data(), request->message_size());
    auto cmd_id = std::make_shared<std::string>(request->command_id());
    auto request_json = std::make_shared<nlohmann::json>(nlohmann::json::parse(payload.c_str()));
    auto type = (*request_json)["Type"].get<DispenseCommand>();
    auto pkid = std::make_shared<std::string>((*request_json)["Primary_Key_ID"].get<std::string>());
    log_->Info("VmbManager::HandleRequest new request {} with pkid {}", (int)type, *pkid);
    switch(type){
        case DispenseCommand::bag_release:
        {
            int dropoff_lane_id = request_json->value("Dropoff_Lane_Id", -1);
            auto cam = cameras_[dropoff_lane_id];
            BagReleaseResponse response(cmd_id, pkid, 10, false, "Bay not found");
            if(cam == nullptr || dropoff_lane_id < 0){
                log_->Error("No repack bay with id {}", dropoff_lane_id);
            }
            else{
                std::shared_ptr<std::string> lfb_generation = std::make_shared<std::string>(request_json->value("Lfb_Generation", "LFB-3.1"));
                log_->Info("Received handle_bag_release Request on Bay {}, PKID: {}, request_id: {}, bot_generation: {}",
                                        cam->name_, *pkid, *cmd_id, *lfb_generation);
                RepackPerception repack_percep(lfb_generation);
                auto image = cam->GetImageBlocking();
                try {
                    std::tuple<int, bool, std::string> release_bot = repack_percep.is_bot_ready_for_release(image);
                    int success_code = std::get<0>(release_bot);
                    bool is_bag_empty_result = std::get<1>(release_bot);
                    std::string error_description = std::get<2>(release_bot);
                    response = BagReleaseResponse(cmd_id, pkid, success_code, is_bag_empty_result, error_description);
                }
                catch (const std::exception& e){
                    log_->Error("Issue handling bot release request: \n\t{}", e.what());
                    response = BagReleaseResponse(cmd_id, pkid, 10, false, e.what());
                }
            }
            SendResponse(*cmd_id, *response.payload);
        break;
        }
        default:
            log_->Error("VmbManager::HandleRequest unhandled command {}", (int)type);
        break;
    }

}