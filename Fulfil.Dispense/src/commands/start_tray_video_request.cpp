//
// Created by sfburke on 4/21/20.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//

#include "Fulfil.Dispense/commands/start_tray_video_request.h"
#include <Fulfil.Dispense/commands/code_response.h>

using fulfil::dispense::commands::StartTrayVideoRequest;
using fulfil::dispense::commands::CodeResponse;
using fulfil::dispense::commands::DispenseResponse;


StartTrayVideoRequest::StartTrayVideoRequest(std::shared_ptr<std::string> command_id,
                                             std::shared_ptr<std::string> PrimaryKeyID)
{
  /**
   * The command id is still somewhat important here because
   * it is used when filtering out commands in the queue.
   */
  this->command_id = command_id;
  this->PrimaryKeyID = PrimaryKeyID;
}

std::shared_ptr<DispenseResponse> StartTrayVideoRequest::execute()
{
  if(!this->delegate.expired())
  {
    std::shared_ptr<DispenseRequestDelegate> tmp_delegate = this->delegate.lock();
    tmp_delegate->handle_start_tray_video(this->PrimaryKeyID); //calls method in dispense_manager to start save image + depth data from current tray session
  }
  else
  {
    std::cout << "StartTrayVideo Command Delegate Expired" << std::endl;
  }
  //std::cout << "returning StartVideoresponse" << std::endl;
  return std::make_shared<CodeResponse>(this->command_id, 0);
}
