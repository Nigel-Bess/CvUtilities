//
// Created by sfburke on 4/21/20.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//

#include "Fulfil.Dispense/commands/stop_lfb_video_request.h"
#include <Fulfil.Dispense/commands/code_response.h>

using fulfil::dispense::commands::StopLFBVideoRequest;
using fulfil::dispense::commands::CodeResponse;
using fulfil::dispense::commands::DispenseResponse;


StopLFBVideoRequest::StopLFBVideoRequest(std::shared_ptr<std::string> command_id,
                                         std::shared_ptr<std::string> PrimaryKeyID)
{
  /**
   * The command id is still somewhat important here because
   * it is used when filtering out commands in the queue.
   */
  this->request_id = command_id;
  this->PrimaryKeyID = PrimaryKeyID;
}

std::shared_ptr<DispenseResponse> StopLFBVideoRequest::execute()
{
  if(!this->delegate.expired())
  {
    std::shared_ptr<DispenseRequestDelegate> tmp_delegate = this->delegate.lock();
    tmp_delegate->handle_stop_lfb_video(); //calls method in dispense_manager to stop save image + depth data from current LFB session
  }
  else
  {
    std::cout << "StopLFBVideo Command Delegate Expired" << std::endl;
  }
  //std::cout << "returning StopVideoresponse" << std::endl;
  return std::make_shared<CodeResponse>(this->request_id, 0);
}