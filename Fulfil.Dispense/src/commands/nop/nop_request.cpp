//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#include "Fulfil.Dispense/commands/nop/nop_request.h"
#include "Fulfil.Dispense/commands/nop/nop_response.h"

using fulfil::dispense::commands::NopRequest;
using fulfil::dispense::commands::DispenseResponse;
using fulfil::utils::Logger;


NopRequest::NopRequest(std::shared_ptr<std::string> command_id, std::shared_ptr<std::string> PrimaryKeyID)
{
  this->command_id = command_id;
  this->PrimaryKeyID = PrimaryKeyID;
}

std::shared_ptr<DispenseResponse> NopRequest::execute()
{
  if(!this->delegate.expired())
  {
    std::shared_ptr<DispenseRequestDelegate> tmp_delegate = this->delegate.lock();
    return std::make_shared<NopResponse>(this->command_id, 0, tmp_delegate->check_motor_in_position());
  }
  else
  {
    Logger::Instance()->Error("Command Delegate Expired; Vars: HomeMotorRequest");
    return std::make_shared<NopResponse>(this->command_id, 9, false);
  }
}