#include <tuple>
#include <json.hpp>
#include <commands/tcs/tcs_actions.h>

using fulfil::utils::Logger;
using fulfil::dispense::commands::tcs::TCSPerception;
using fulfil::dispense::commands::tcs::TCSActions;
using fulfil::utils::commands::DispenseCommand;

TCSActions::TCSActions(TCSPerception* tcs_perception) {
    this->tcs_perception = tcs_perception;
}

std::shared_ptr<DcResponse> TCSActions::handle_pre_pick_clip_actuator_request(cv::Mat color_bag_clips_img, std::string &pkid, std::string &cmd_id) {
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

    auto stopTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - start_time).count();
    Logger::Instance()->Info("handle_pre_pick_clip_actuator_request: id={} took {}ms", pkid, duration);
    return response;

    //auto clip_states = tcs_perception->getBagClipStates(bag_image, lfr_version, pkid, directory_path);
    //return clip_states;
}

std::shared_ptr<DcResponse> TCSActions::to_response(std::string &cmd_id, std::string &response){
    auto api_resp = std::make_shared<DcResponse>();
    api_resp->set_command_id(cmd_id);
    Logger::Instance()->Debug("Response id: {}", api_resp->command_id());
    api_resp->set_type(MESSAGE_TYPE_GENERIC_QUERY);
    api_resp->set_message_size(response.size());
    api_resp->set_message_data(response.data(), response.size());
    return api_resp;
}
