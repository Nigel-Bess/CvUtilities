//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DROP_TARGET_DROP_TARGET_RESPONSE_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DROP_TARGET_DROP_TARGET_RESPONSE_H_
#include <Fulfil.Dispense/commands/dispense_response.h>
#include <Fulfil.CPPUtils/logging.h>

namespace fulfil
{
namespace dispense {
namespace commands
{
/**
 * The purpose of this class is to outline and contain
 * all of the information required for the response to
 * a drop target response
 */
class DropTargetResponse : public fulfil::dispense::commands::DispenseResponse
{
 private:

  /**
   * The id for the request.
   */
  std::shared_ptr<std::string> command_id;
  /**
   * The x offset target result from the center of the LFB bag, in dispense system coordinates, mm units
   */
  float rover_position;
  /**
   * The y offset target result from the center of the LFB bag, in dispense system coordinates, mm units
   */
  float dispense_position;
  /**
   * depth_result Z coordinate of depth target relative to aruco plane on top of LFB, mm units
   */
  float depth_result;
  /**
   * depth of tallest item in LFB bag, in mm, relative to the top surface of the bag
   */
  float max_Z;
  /**
   * Indicates that target is being sent to VLSG with understanding that LFB should be rotated 180 degrees FROM IT'S CURRENT STATE before the dispense
   */
  bool Rotate_LFB;
  /**
   * Indicates that CV detected the bot as rotated 180 degrees (= 1) or nominal orientation (= 0) or unsure (= -1)
   */
  bool LFB_Currently_Rotated;
  /**
   * Indicates that dispensed item is expected to collide with item already in bag during swing part of the dispense
   */
  bool Swing_Collision_Expected;
  /**
   *  success_code = 0 if successful, > 0 if there was an error
   */
  int success_code;
  /**
   * Description of the error code thrown. Will be empty string if code is success.
   */
  std::string error_description;
  /**
   * The payload to be sent in response to the request
   */
  std::shared_ptr<std::string> payload;
  /**
   * Encodes the payload in the payload string variable on this object.
   */
  void encode_payload();
 public:
  /**
   * DropTargetResponse constructor that initializes a response indicating a failure.
   * @param command_id the command id of the request that led to this response
   * @param success_code indicates what kind of error took place. > 0 means an error
   */
  explicit DropTargetResponse(std::shared_ptr<std::string> command_id, int success_code, std::string error_description);
  /**
   * DropTargetResponse constructor that initializes a response including target information
   * @param command_id of the request that led to this response.
   * See above for descriptions of the other params
   */
  DropTargetResponse(std::shared_ptr<std::string> command_id, int success_code, float rover_position, float dispense_position,
                     float depth_result, float max_Z, bool Rotate_LFB, bool LFB_Currently_Rotated, bool Swing_Collision_Expected, std::string error_description);
  /**
   * Returns the command id for the response.
   * @return pointer to string containing command id for the response.
   */
  std::shared_ptr<std::string> get_command_id() override;
  /**
   * Returns the size (in bytes) of the payload containing information about the drop result.
   * @return size (in bytes) of the payload to be sent.
   */
  int dispense_payload_size() override;
  /**
   * Returns the payload containing information about the drop result.
   * @return pointer to string containing data representing the drop result.
   */
  std::shared_ptr<std::string> dispense_payload() override;
};
} // namespace commands
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DROP_TARGET_DROP_TARGET_RESPONSE_H_
