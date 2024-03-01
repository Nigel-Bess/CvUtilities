//
// Created by steve on 5/12/20.
//

#include <Fulfil.Dispense/commands/tray_validation/tray_validation_details.h>
#include <iostream>
#include <Fulfil.Dispense/tray/tray_lane.h>

using fulfil::dispense::commands::TrayValidationDetails;
using fulfil::dispense::tray::TrayLane;

TrayValidationDetails::TrayValidationDetails(std::shared_ptr<std::string> request_id, std::shared_ptr<nlohmann::json> request_json)
{
  //this->rigid =  (*request_json)["Rigid"].get<bool>();
  this->height_tolerance = (*request_json)["Height_Tolerance"].get<double>();

  auto json_lanes = (*request_json)["Lanes"];

  std::cout << json_lanes << std::endl;

  std::shared_ptr<std::vector<std::shared_ptr<TrayLane>>> tray_lanes = std::make_shared<std::vector<std::shared_ptr<TrayLane>>>();
  int lane_num = 1;

  for (auto item_json = json_lanes.begin(); item_json != json_lanes.end(); ++item_json)
  {
    auto lane = item_json.value();
    auto item = lane["Item"];

    double item_height = item["H"];
    double item_length = item["L"];
    double item_width = item["W"];
    int num_items = lane["Num_Items"];
    bool rigid = lane["Rigid"].get<int>();
    bool has_tongue = lane["Rigid"].get<bool>();

    //std::cout << "lane:" << lane << std::endl;
    //std::cout << item_height << item_length << item_width << num_algo_counts << rigid << std::endl;

    std::shared_ptr<TrayLane> tray_lane = std::make_shared<TrayLane>(lane_num, item_height, item_length, item_width, num_items, rigid, has_tongue);
    tray_lanes->push_back(tray_lane);
    lane_num ++;
  }

  this->tray_lanes = tray_lanes;
}
