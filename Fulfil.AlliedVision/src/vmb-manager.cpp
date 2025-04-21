#include "vmb-manager.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <tuple>
#include "commands/bag_release/repack_perception.h"
#include "commands/bag_release/bag_release_response.h"
#include <filesystem>
#include "commands/bag_release/repack_error_codes.h"
#include <chrono>
#include <Fulfil.CPPUtils/commands/dispense_command.h>

using fulfil::utils::commands::DispenseCommand;
using fulfil::dispense::commands::BagReleaseResponse;
using fulfil::dispense::commands::RepackPerception;
using fulfil::dispense::commands::RepackErrorCodes;
using fulfil::dispense::commands::get_error_name_from_code;

const int MAX_ACTIVE_CAMERAS = 2;

struct sigaction old_action;
bool RUN = true;

void sig_handler(int sig_no){
    printf("Graceful termination requested");
    RUN = false;
}

VmbManager::VmbManager(std::map<int, std::string> cam_map, fulfil::utils::Logger* logger){
    log_ = logger;
    service_ = std::shared_ptr<GrpcService>(new GrpcService());
    for(auto const & [bay, ip] : cam_map){
        log_->Info("Setting up new VmbCamera on Repack Bay {} with IP:{}",bay ,ip);
        cameras_.emplace(bay, std::make_shared<VmbCamera>(ip, bay, log_, service_));
    }
    active_cams = 0;
    RunManager();//block
    std::thread(&VmbManager::RunManager, this).detach();
}

void VmbManager::RunManager(){
    if(!RUN)return;
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = &sig_handler;
    sigaction(SIGINT, &action, &old_action);
    // Applies the perfect ctrl+C teardown logic to the K8s-friendly SIGTERM signal as well so the entirety of this service will be Kubernetes friendly for the whole lifecycle
    sigaction(SIGTERM, &action, &old_action);

    log_->Info("=== VmbManager v1.1 starting ===\n");

    auto code = vmb_system.Startup();
    if(code != VmbErrorSuccess){
        log_->Error("Error starting up VmbSystem, failed with code {}!", GetVimbaCode(code));
        exit(1);
    }

    log_->Info("Querying network for discovered cameras\n");
    CameraPtrVector discovered;
    code = vmb_system.GetCameras(discovered);
    log_->Info("Vimba discovered cam count: {}, expected: {}\n", (int)discovered.size(), (int)cameras_.size());

    if(code != VmbErrorSuccess){
        log_->Error("Error GetCameras, failed with code {}!", GetVimbaCode(code));
    }

    log_->Info("VmbManager startup successful");
    auto found = 0;
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
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    if(!found){
        log_->Error("No cameras found, exiting");
        RUN = false;
    }
    service_->Run(9395);
    log_->Info("{} Cameras initialized", found);
    while (RUN){
        // Throttle queue handling slightly
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        if(!service_->IsConnected())continue;
        if(!service_->HasNewRequest())continue;
        auto request = service_->GetNextRequest();
        std::thread(&VmbManager::HandleRequest, this, request, 1).detach();
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

/* Returns true if image saving was successful, false otherwise */
bool VmbManager::SaveImages(cv::Mat bag_image, std::string image_path) {
    try {
        std::string img_name = image_path + ".jpeg";
        if (bag_image.size().empty()) {
            log_->Error("Cannot save empty image to {}", img_name);
            return false;
        }
        cv::imwrite(img_name, bag_image);
        log_->Info("{} saved successfully!!!", img_name);
        return true;

    }
    catch (const std::exception& ex) {
        log_->Error("VmbManager::SaveImages caught error: {}", ex.what());
        return false;
    }
    catch (...) {
        log_->Error("VmbManager::SaveImages hit error in catch(...)");
        return false;
    }
}

void VmbManager::HandleRequest(std::shared_ptr<DepthCameras::DcRequest> request, int remainingRetries){
    auto startTime = std::chrono::steady_clock::now();
    
    std::string payload(request->message_data().data(), request->message_size());
    auto cmd_id = std::make_shared<std::string>(request->command_id());
    auto request_json = std::make_shared<nlohmann::json>(nlohmann::json::parse(payload.c_str()));
    auto type = (*request_json)["Type"].get<DispenseCommand>();
    auto pkid = std::make_shared<std::string>((*request_json)["Primary_Key_ID"].get<std::string>());
    log_->Info("VmbManager::HandleRequest new request {} with pkid {} and payload: {}", (int)type, *pkid, payload);
    switch(type){
        case DispenseCommand::bag_release:
        {
            int dropoff_lane_id = request_json->value("Dropoff_Lane_Id", -1);
            auto cam = cameras_[dropoff_lane_id];
            bool always_approve_for_release = request_json->value("Always_Approve_For_Release", false);
            BagReleaseResponse response(cmd_id, pkid, always_approve_for_release, 10, false, "Bay not found");
            if(cam == nullptr || dropoff_lane_id < 0){
                log_->Error("No repack bay with id {}", dropoff_lane_id);
            }
            else{
                std::shared_ptr<std::string> lfb_generation = std::make_shared<std::string>(request_json->value("Lfb_Generation", "LFB-3.1"));
                log_->Info("Received handle_bag_release Request on Bay {}, PKID: {}, request_id: {}, bot_generation: {}",
                                        cam->name_, *pkid, *cmd_id, *lfb_generation);
                RepackPerception repack_percep(lfb_generation);
                // Lazy, ideally this would be a counting_semaphor but getting to CPP 20 was too difficult to be worth it :-(
                while (active_cams >= MAX_ACTIVE_CAMERAS) {
                    log_->Info("Stalling GetImageBlocking, {} cameras currently active already", active_cams);
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
                active_cams++;
                auto image = cam->GetImageBlocking();
                active_cams--;
                cv::Mat cam_image = *image;
                std::string directory_path = "/home/fulfil/data/" + std::string(*pkid) + "/";
                if (!std::filesystem::exists(directory_path)) {
                    if (std::filesystem::create_directories(directory_path)) {
                        log_->Info("Directory created : {}", directory_path);
                        std::string image_path = directory_path + "color_image";
                        bool saved_image_successfully = SaveImages(cam_image, image_path);
                        if (!saved_image_successfully) {
                            repack_percep.success_code = RepackErrorCodes::UnspecifiedError;
                            repack_percep.error_description = "Failed to save image successfully, likely the image was empty!";
                        }
                    }
                    else {
                        log_->Error("Failed to create Directory : {}", directory_path);
                        repack_percep.success_code = RepackErrorCodes::UnspecifiedError;
                        repack_percep.error_description = "Failed to create Directory : " + directory_path;
                    }
                }
                
                try {
                    // don't run the algorithm if the setup encountered issues
                    if (repack_percep.success_code == RepackErrorCodes::Success)
                    {
                        log_->Info("About to run is_bot_ready_for_release");
                        int lfbId = request_json->value("Lfb_Id", 0);
                        repack_percep.is_bot_ready_for_release(image, *pkid, cam->name_, lfbId, directory_path);
                        log_->Info("Results from is_bot_ready_for_release are success code: {}, is_bag_empty: {}, error_description: {}", 
                            repack_percep.success_code, 
                            repack_percep.is_bag_empty, 
                            repack_percep.error_description);
                        // Re-run auto exposure just in case if no markers were found
                        if (remainingRetries > 0 && repack_percep.success_code == RepackErrorCodes::NoMarkersDetected) {
                            log_->Warn("No markers found at {}, trying to reset exposure", cam->name_);
                            cam->RunAutoExposure();
                            return this->HandleRequest(request, remainingRetries-1);
                        }
                    } else 
                    {
                        if (cam->camera_error_code != RepackErrorCodes::Success) {
                            repack_percep.success_code = cam->camera_error_code;
                            repack_percep.error_description = get_error_name_from_code((RepackErrorCodes)repack_percep.success_code) + " -> " +  cam->camera_error_description;
                        } 
                        log_->Debug("RepackPerception success code is {}, Not going to proceed with is_bot_ready_for_release", repack_percep.success_code);
                    }
                    
                    response = BagReleaseResponse(cmd_id, pkid, always_approve_for_release, repack_percep.success_code, repack_percep.is_bag_empty, repack_percep.error_description);
                }
                catch (const std::exception& e){
                    log_->Error("Exception caught in VmbManager. Issue handling bot release request: \n\t{}", e.what());
                    response = BagReleaseResponse(cmd_id, pkid, always_approve_for_release, RepackErrorCodes::UnspecifiedError, false, e.what());
                }
            }
            auto stopTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime).count();
            log_->Info("HandleRequest {} took {}ms on {}", *pkid, duration, cam->name_);
            SendResponse(*cmd_id, *response.payload);
        break;
        }
        default:
            log_->Error("VmbManager::HandleRequest unhandled command {}", (int)type);
    }

}