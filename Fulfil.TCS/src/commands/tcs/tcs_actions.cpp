#include <tuple>
#include <json.hpp>
#include <commands/tcs/tcs_actions.h>

using fulfil::utils::Logger;
using fulfil::dispense::commands::tcs::TCSPerception;
using fulfil::dispense::commands::tcs::TCSActions;

TCSActions::TCSActions(TCSPerception* tcs_perception) {
    this->tcs_perception = tcs_perception;
}

std::shared_ptr<nlohmann::json> TCSActions::handle_pre_pick_clip_actuator_request(cv::Mat color_bag_clips_img, std::shared_ptr<nlohmann::json> request, std::string &cmd_id) {
    // TODO remove all lfr version stuff
    auto start_time = std::chrono::steady_clock::now();
    auto pkid = (*request)["Primary_Key_ID"].get<std::string>();
    auto tote_id = (*request)["Tote_Id"].get<int>();
    auto facility_id = (*request)["Facility_Id"].get<int>();
    auto bag_cavity_index = (*request)["Bag_Cavity_Index"].get<int>();
    auto expected_bag_type = (*request)["Expected_Bag_Type"].get<int>();
    
    std::string lfr_version = "LFB-3.2";

    auto clipsState = tcs_perception->getBagClipStates(color_bag_clips_img, "LFP", pkid, "/home/fulfil/data/bag_clip");
    
    std::shared_ptr<nlohmann::json> result_json = std::make_shared<nlohmann::json>();
    (*result_json)["Error"] = 0;
    (*result_json)["Error_Description"] = "";
    (*result_json)["Tote_Id"] = tote_id;
    (*result_json)["Primary_Key_ID"] = pkid;
    (*result_json)["Facility_Id"] = facility_id;
    (*result_json)["Clip_Open_States"] = nlohmann::json::parse("[true, true, true, true]");

    auto stopTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - start_time).count();
    Logger::Instance()->Info("handle_pre_pick_clip_actuator_request: id={} took {}ms", pkid, duration);
    return result_json;
}
