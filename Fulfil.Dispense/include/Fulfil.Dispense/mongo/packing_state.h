//
// Created by steve on 3/17/21.
//

#ifndef FULFIL_DISPENSE_SRC_DROP_PACKING_STATE_H_
#define FULFIL_DISPENSE_SRC_DROP_PACKING_STATE_H_

#include <memory>
#include <vector>
#include <opencv2/opencv.hpp>
#include <Fulfil.CPPUtils/point_3d.h>
#include <Fulfil.CPPUtils/pixel.h>
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include <Fulfil.CPPUtils/logging.h>
#include <json.hpp>

namespace fulfil
{
namespace dispense {
namespace drop
{
/**
 * The purpose of this class is to provide functions for the analysis of the state of the LFB bag's packing status
 * including the current occupied volume beneath items already dispensed into the bag, remaining available volume
 * for new items, and the associated bag packing efficiency.
 *
**/

class PackingState
{
 private:

  // number of potential max depth detections for given LFB bag
  int max_num_depth_detections;

  //the depth of the bag cavity, in meters (e.g. -0.32)
  float bag_cavity;

  /**
   *  this is the "volume" of the bag when empty. Approximated by the bag depth * num_depth_detections
   *  this is not reflective of the actual volume in the bag in m^3.
   *  Note that the bag depths used will be in meter units.
   */
  float empty_bag_volume_points;

   // the volume of the empty bag in mm^3 units. Defined by LFB configs
  float empty_bag_volume_mm;

  // the total volume of all items already packed in the bag, in mm^3 units. Tracked in mongo
  float packed_items_volume_mm;

  // The percentage of the bag's "volume" that is currently taken up by packed items. Tracked in mongo. To be returned to VLSG
  int percent_bag_full;

  // the total "volume" taken up in the bag by all items that have been dispensed. Tracked in mongo. To be returned to VLSG
  int packing_efficiency;


 public:
  /**
   * PackingState constructor
   */
  PackingState(float LFB_cavity_height, float container_width, float container_length, int max_num_depth_detections);

  void set_packed_items_volume_mm(float value);

  // update the packed items volume with the input dimensions (in meters) of the new item added to the bag
  void update_packed_items_volume(std::shared_ptr<nlohmann::json> drop_request_json);

  float get_packed_items_volume_mm();

  int get_percent_bag_full();

  int get_packing_efficiency();

  int update_detected_volume(std::shared_ptr<Eigen::Matrix3Xd> point_cloud_data, float remaining_platform);

};
} // namespace drop
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_SRC_DROP_PACKING_STATE_H_
