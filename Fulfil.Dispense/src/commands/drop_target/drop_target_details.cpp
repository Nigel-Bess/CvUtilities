//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#include <iostream>
#include <Fulfil.Dispense/commands/drop_target/drop_target_details.h>

using fulfil::dispense::commands::DropTargetDetails;


float DropTargetDetails::to_meters(float millimeters)
{
  return millimeters/1000.0F;
}
uint32_t DropTargetDetails::to_millimeters(double meters)
{
  return (uint32_t)(round(meters * 1000));
}

DropTargetDetails::DropTargetDetails(std::shared_ptr<nlohmann::json> request_json,
                                         std::shared_ptr<std::string> command_id)
{
  this->request_id = request_id;
  this->bag_id = (*request_json)["Bag_ID"].get<std::string>();
  this->bag_item_count = (*request_json)["Bag_Item_Count"].get<int>();
  // convert the item dimension from mm to meters
  this->item_length = DropTargetDetails::to_meters((*request_json)["Lanes"].begin().value()["Item"]["L"].get<float>());
  this->item_width = DropTargetDetails::to_meters((*request_json)["Lanes"].begin().value()["Item"]["W"].get<float>());
  this->item_height = DropTargetDetails::to_meters((*request_json)["Lanes"].begin().value()["Item"]["H"].get<float>());
  this->item_mass = (*request_json)["Lanes"].begin().value()["Item"]["Mass"].get<float>(); // [grams]
  this->item_shiny = (*request_json)["Lanes"].begin().value()["Item"]["Shiny"].get<bool>();
  this->limit_left = (*request_json)["Limit_Left"].get<int>();
  this->limit_right = (*request_json)["Limit_Right"].get<int>();
  this->limit_front = (*request_json)["Limit_Front"].get<int>();
  this->limit_back = (*request_json)["Limit_Back"].get<int>();
  this->remaining_platform = DropTargetDetails::to_meters((*request_json)["Remaining_Platform"].get<float>());
  this->use_flipped_x_default = request_json->value("Flip_X_Default", false);

  //handling of damage code based on material and fragile properties. See bag state tracking + damage handling design doc
  this->item_material = (*request_json)["Lanes"].begin().value()["Item"]["Material"].get<int>();
  int item_fragile =  (*request_json)["Lanes"].begin().value()["Item"]["Fragility"].get<int>(); // 0 = not fragile, 1 = fragile, 2 = extra fragile

  //note: glass and metal items should never be designated as fragile. Will not be handled as fragile
  if(item_fragile == 1 and this->item_material != 4 and this->item_material != 3)
  {
    this->item_damage_code = 20; //Fragile damage avoidance designation
  }
  else if (item_fragile == 2)
  {
    this->item_damage_code = 21; //Extra fragile damage avoidance designation
  }
  else
  {
    this->item_damage_code = this->item_material;
  }
}
