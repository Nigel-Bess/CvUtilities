//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#include <Fulfil.Dispense/commands/drop_target/drop_target_request.h>
#include <Fulfil.Dispense/commands/dispense_request_delegate.h>
#include <Fulfil.Dispense/commands/drop_target/drop_target_response.h>
#include "Fulfil.Dispense/dispense/drop_error_codes.h"
#include <Fulfil.DepthCam/visualization.h>


using fulfil::dispense::commands::DropTargetRequest;
using fulfil::dispense::commands::DropTargetDetails;
using fulfil::dispense::commands::DispenseRequestDelegate;
using fulfil::dispense::drop_target_error_codes::DropTargetErrorCodes;


DropTargetRequest::DropTargetRequest(std::shared_ptr<std::string> command_id,
                                     std::shared_ptr<std::string> PrimaryKeyID,
                                     std::shared_ptr<DropTargetDetails> details,
                                     std::shared_ptr<nlohmann::json> request_json)
{
  this->command_id = command_id;
  this->PrimaryKeyID = PrimaryKeyID;
  this->details = details;
  this->request_json = request_json;
};

std::shared_ptr<fulfil::dispense::commands::DispenseResponse> fulfil::dispense::commands::DropTargetRequest::execute()
{
  if(!this->delegate.expired())
  {
    std::shared_ptr<DispenseRequestDelegate> tmp_delegate = this->delegate.lock();
    //gets DropResult by calling method in dispense_manager
    std::shared_ptr<fulfil::dispense::drop::DropResult> raw_result = tmp_delegate->handle_drop_target(this->details,
                                                                                                       this->request_json);
    if(raw_result->success_code == 0 or raw_result->success_code == 9)
    {
      //initialize and return a DropTargetResponse based on the DropResult
      return std::make_shared<fulfil::dispense::commands::DropTargetResponse>(this->command_id, raw_result->success_code,
          raw_result->rover_position, raw_result->dispense_position, raw_result->depth_result, raw_result->max_Z,
          raw_result->Rotate_LFB, raw_result->LFB_Currently_Rotated, raw_result->Swing_Collision_Expected, raw_result->error_description);
    }
    else
    {
      return std::make_shared<fulfil::dispense::commands::DropTargetResponse>(this->command_id, raw_result->success_code, raw_result->error_description);
    }
  }
  else
  {
    std::cout << "DropTarget Command Delegate Expired" << std::endl;
    return std::make_shared<fulfil::dispense::commands::DropTargetResponse>(this->command_id, DropTargetErrorCodes::CommandDelegateExpired, "DispenseRequestDelegate expired");
  }
}


