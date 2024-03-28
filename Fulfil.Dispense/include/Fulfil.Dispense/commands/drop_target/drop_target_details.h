//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DROP_TARGET_DETAILS_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DROP_TARGET_DETAILS_H_
#include <memory>
#include <Fulfil.Dispense/commands/drop_target/json_item.h>
#include <iostream>

namespace fulfil
{
namespace dispense {
namespace commands
{
/**
 * The purpose of this class is to contain all of the required
 * information for processing a drop target request with a
 * sensor.
 */
class DropTargetDetails
{
 private:
  /**
   * Converts millimeters to meters.
   * @param millimeters to be converted.
   * @return meters equivalent of provided millimeters.
   */
  float to_meters(float millimeters);
  /**
   * Converts millimeters to meters.
   * @param meters to be converted.
   * @return millimeters equivalent of provided meters.
   */
  uint32_t to_millimeters(double meters);

 public:
  /**
   * DropTargetDetails constructor.
   */
  DropTargetDetails(std::shared_ptr<nlohmann::json> request_json,
                    std::shared_ptr<std::string> command_id);
  /**
   * A std::string containing the id for the request.
   */
  std::shared_ptr<std::string> request_id;

  /**
   * The mongo id for the LFB bag of interest
   */
  std::string bag_id;

  /**
  * Number of items in LFB bag before the dispense
  */
  int bag_item_count;

  /**
   * The length of the item in the tray being dispensed in meters
   */
  float item_length;

  /**
   * The width of the item in the tray being dispensed in meters
   */
  float item_width;

  /**
   * The height of the item in the tray being dispensed in meters
   */
  float item_height;

  /**
  * Mass of item being dropped (in grams)
  */
  float item_mass;

  /**
   * Material property of item
   */
  int item_material;

  /**
   *   code for the damage type of the item (see bag state tracking design doc).
   *   generally matches material code, and then fragile = 20, extra fragile = 21
   */
  int item_damage_code;

  /**
   *   boolean for whether the item should be treated as "shiny" --> likely to cause depth detection issues
   */
  bool item_shiny;

  /**
   *  Amount (in mm) that target search should be limited, from Left side of LFB
   */
  int limit_left;

  /**
   *  Amount (in mm) that target search should be limited, from Right side of LFB
   */
  int limit_right;

  /**
  *  Amount (in mm) that target search should be limited, from Front side of LFB
  */
  int limit_front;

  /**
   *  Amount (in mm) that target search should be limited, from Back side of LFB
   */
  int limit_back;

  /**
   * The width of the tongue in the lane being dispensed from in meters
   */
  float tongue_width;

  /**
   *  Amount (in m) that the LFB platform can lower before bottoming out
   */
  float remaining_platform;

  /**
   *  Enables flipping the default side for the drop target, from Rear to Bow
   */
  bool use_flipped_x_default;
};
} // namespace commands
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DROP_TARGET_DETAILS_H_
