//
// Created by steve on 3/17/21.
//

#include "Fulfil.Dispense/mongo/packing_state.h"

using fulfil::dispense::drop::PackingState;
using fulfil::utils::Logger;

PackingState::PackingState(float LFB_cavity_height, float container_width, float container_length, int max_num_depth_detections)//default to 3.1
{
  // the max number of depth detections in the LFB container by the point cloud, when there are no missing depth data points (will vary some from image/image, bay/bay, etc.)
  this->max_num_depth_detections = max_num_depth_detections; //1250; // = 25 x 35
  this->bag_cavity = LFB_cavity_height;

  this->empty_bag_volume_points = this->bag_cavity * this->max_num_depth_detections;
  this->empty_bag_volume_mm = (this->bag_cavity*1000) * (container_width *1000) * (container_length*1000);

  this->packed_items_volume_mm = 0; //this is 0 so new bags will be initialized with this value
  this->percent_bag_full = 0;
  this->packing_efficiency = -1;
}

void PackingState::set_packed_items_volume_mm(float value)
{
  this->packed_items_volume_mm = value;
}

void PackingState::update_packed_items_volume(std::shared_ptr<nlohmann::json> drop_request_json)
{
  if(this->packed_items_volume_mm == -1)
  {
    Logger::Instance()->Error("Cannot update packed items volume because do not know current state of packed item volume!");
    return;
  }
  else
  {
    float item_length = (*drop_request_json)["Lanes"].begin().value()["Item"]["L"].get<float>();
    float item_width = (*drop_request_json)["Lanes"].begin().value()["Item"]["W"].get<float>();
    float item_height = (*drop_request_json)["Lanes"].begin().value()["Item"]["H"].get<float>();
    int quantity = (*drop_request_json)["Lanes"].begin().value()["Num_Items"].get<int>();; //TODO: if move towards multi-quantity dispenses, need to update this accordingly.

    float previous_volume = this->packed_items_volume_mm;
    float new_volume = previous_volume + (item_length * item_width * item_height * quantity); //TODO: can't assume correct quantity was dispensed. This needs to be set by VLSG eventually
    this->packed_items_volume_mm = new_volume;
    Logger::Instance()->Debug("Updated bag packing state, packed_items_volume_mm from {} to {}", previous_volume, new_volume);
  }

  /**
   * Also update packing efficiency! TODO: change name of method to better capture this
   */
  Logger::Instance()->Debug("Total item volume {} vs. actual detected taken up volume {}", this->packed_items_volume_mm,
                           (this->empty_bag_volume_mm * this->percent_bag_full / 100));

  this->packing_efficiency = int((this->packed_items_volume_mm) * 100 / (this->empty_bag_volume_mm * this->percent_bag_full / 100));
  Logger::Instance()->Debug("Resulting bag packing efficiency is: {}", this->packing_efficiency);
}

float PackingState::get_packed_items_volume_mm()
{
  return this->packed_items_volume_mm;
}

int PackingState::get_percent_bag_full()
{
  return this->percent_bag_full;
}

int PackingState::get_packing_efficiency()
{
  return this->packing_efficiency;
}

int PackingState::update_detected_volume(std::shared_ptr<Eigen::Matrix3Xd> point_cloud_data, float remaining_platform)
{
  int num_detections = point_cloud_data->cols();
  Logger::Instance()->Debug("Updating packing_state with point cloud data of size: {}", num_detections);

  float points_volume_sum = 0;
  int valid_point_count = 0;

  for (int i = 0; i < num_detections; i++)
  {
    float x_value = (*point_cloud_data)(0,i);
    float y_value = (*point_cloud_data)(1,i);
    float depth_value = (*point_cloud_data)(2,i);

    //TODO: a filtered point cloud should be provided to this function in the future, once container refactor happens
    if(depth_value < (-1*this->bag_cavity) or depth_value > this->bag_cavity or abs(x_value) > (0.32/2.0) or abs(y_value) > (0.23/2.0))
    {
      Logger::Instance()->Trace("Point cloud value of {}, {}, {} appears to be out or range", x_value, y_value, depth_value);
    }
    else
    {
      points_volume_sum += depth_value;
      valid_point_count++;
    }
  }

  if(valid_point_count > this->max_num_depth_detections)
  {
    Logger::Instance()->Warn("More valid points detected ({}) than expected ({}). Will still move forward w/ updating packing state", valid_point_count, max_num_depth_detections);
  }
  if(remaining_platform > this->bag_cavity)
  {
    Logger::Instance()->Warn("More remaining platform movement ({}) than bag_cavity in LFB_config.ini ({}). Check configs. Will result in invalid BPE values", remaining_platform, this->bag_cavity);
  }
  if(valid_point_count < 1)
  {
    Logger::Instance()->Error("No valid points detected in point cloud... will not update packing state");
    return -1;
  }

  points_volume_sum = points_volume_sum * -1; //convert the "volume" to a positive value corresponding to remaining available space in bag
  Logger::Instance()->Trace("The total sum of detected depth point values in bag is: {}", points_volume_sum);

  float average_remaining_volume = (points_volume_sum / valid_point_count) + remaining_platform;
  int percent_remaining_volume = int(100 * average_remaining_volume / this->bag_cavity);
  Logger::Instance()->Debug("Percent of bag remaining, based on point cloud volume: {}", percent_remaining_volume);

  int percent_bag_full = 100 - percent_remaining_volume;
  this->percent_bag_full = percent_bag_full;
  return percent_bag_full;
}
