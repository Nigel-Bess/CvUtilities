//
// Created by steve on 12/30/21.
// Copyright (c) 2021 Fulfil Solutions, Inc. All rights reserved.
//

#include "Fulfil.Dispense/commands/home_motor_request.h"
#include <Fulfil.Dispense/commands/code_response.h>
#include <Fulfil.CPPUtils/logging.h>

using fulfil::dispense::commands::HomeMotorRequest;
using fulfil::dispense::commands::CodeResponse;
using fulfil::dispense::commands::DispenseResponse;
using fulfil::utils::Logger;


HomeMotorRequest::HomeMotorRequest(std::shared_ptr<std::string> command_id, std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json)
{
  /**
   * The command id is still somewhat important here because it is used when filtering out commands in the queue.
   */
  this->command_id = command_id;
  this->PrimaryKeyID = PrimaryKeyID;
  this->request_json = request_json;
}

std::shared_ptr<DispenseResponse> HomeMotorRequest::execute()
{
  if(!this->delegate.expired())
  {
    std::shared_ptr<DispenseRequestDelegate> tmp_delegate = this->delegate.lock();

    int error_code = tmp_delegate->handle_home_motor(this->PrimaryKeyID, this->request_json);

    return std::make_shared<CodeResponse>(this->command_id, error_code);
  }
  else
  {
    Logger::Instance()->Error("Command Delegate Expired; Vars: HomeMotorRequest");
    return std::make_shared<CodeResponse>(this->command_id, 9);
  }
}
