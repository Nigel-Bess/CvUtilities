//
// Created by sfburke on 4/21/20.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//

#include "Fulfil.Dispense/commands/video/stop_tray_video_request.h"
#include <Fulfil.Dispense/commands/code_response.h>
#include <Fulfil.CPPUtils/logging.h>

using fulfil::utils::Logger;
using fulfil::dispense::commands::StopTrayVideoRequest;
using fulfil::dispense::commands::CodeResponse;
using fulfil::dispense::commands::DispenseResponse;


StopTrayVideoRequest::StopTrayVideoRequest(std::shared_ptr<std::string> command_id,
                                           std::shared_ptr<std::string> PrimaryKeyID,
                                           std::shared_ptr<nlohmann::json> request_json)
{
  /**
   * The command id is still somewhat important here because
   * it is used when filtering out commands in the queue.
   */
  this->request_id = command_id;
  this->PrimaryKeyID = PrimaryKeyID;
  this->request_json = request_json;
}

std::shared_ptr<DispenseResponse> StopTrayVideoRequest::execute()
{
  if(!this->delegate.expired())
  {
    int delay = 0;
    if ((*this->request_json).contains("Delay_Ms")){
        delay = std::min(5000, (*this->request_json)["Delay_Ms"].get<int>());
        //usleep(1000*delay);
        Logger::Instance()->Debug("Received stop video delay of {} ms", delay);
    }
    std::shared_ptr<DispenseRequestDelegate> tmp_delegate = this->delegate.lock();
    tmp_delegate->handle_stop_tray_video(delay); //calls method in dispense_manager to stop save image + depth data from current tray session
  }
  else
  {
    std::cout << "StopTrayVideo Command Delegate Expired" << std::endl;
  }
  Logger::Instance()->Debug("Sending stop tray video response now");
  return std::make_shared<CodeResponse>(this->request_id, 0);
}