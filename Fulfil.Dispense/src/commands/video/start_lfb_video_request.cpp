//
// Created by sfburke on 4/21/20.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//

#include "Fulfil.Dispense/commands/video/start_lfb_video_request.h"
#include <Fulfil.Dispense/commands/code_response.h>

using fulfil::dispense::commands::StartLFBVideoRequest;
using fulfil::dispense::commands::CodeResponse;
using fulfil::dispense::commands::DispenseResponse;


StartLFBVideoRequest::StartLFBVideoRequest(std::shared_ptr<std::string> command_id,
                                           std::shared_ptr<std::string> PrimaryKeyID)
{
  /**
   * The command id is still somewhat important here because
   * it is used when filtering out commands in the queue.
   */
  this->request_id = command_id;
  this->PrimaryKeyID = PrimaryKeyID;
}

std::shared_ptr<DispenseResponse> StartLFBVideoRequest::execute()
{
  if(!this->delegate.expired())
  {
    std::shared_ptr<DispenseRequestDelegate> tmp_delegate = this->delegate.lock();
    tmp_delegate->handle_start_lfb_video(this->PrimaryKeyID); //calls method in dispense_manager to start save image + depth data from current LFB session
  }
  else
  {
    std::cout << "StartLFBVideo Command Delegate Expired" << std::endl;
  }
  //std::cout << "returning StartVideoresponse" << std::endl;
  return std::make_shared<CodeResponse>(this->request_id, 0);
}
