//
// Created by steve on 12/30/21.
// Copyright (c) 2021 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_HOME_MOTOR_REQUEST_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_HOME_MOTOR_REQUEST_H_

#include <Fulfil.Dispense/commands/dispense_request.h>
#include <Fulfil.Dispense/commands/dispense_response.h>

namespace fulfil::dispense::commands
{
/**
 * The purpose of this class is to provide an implementation
 * of a request for homing the motor for the LFB camera on rail
 */
class HomeMotorRequest : public fulfil::dispense::commands::DispenseRequest
{
 public:
  std::shared_ptr<nlohmann::json> request_json;
  /**
   * HomeMotor constructor.
   * @param command_id of the request.
   */
  explicit HomeMotorRequest(std::shared_ptr<std::string> command_id, std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json);
  /**
   * Calls method in dispense_manager to start saving data from current LFB session
   */
  std::shared_ptr<fulfil::dispense::commands::DispenseResponse> execute() override;
};
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_HOME_MOTOR_H_
