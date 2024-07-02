//
// Created by sfburke on 4/21/20.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//

#include <Fulfil.CPPUtils/logging.h>
#include "Fulfil.Dispense/commands/post_drop/post_LFR_request.h"
#include <Fulfil.Dispense/commands/post_drop/post_LFR_response.h>


using fulfil::utils::Logger;
using fulfil::dispense::commands::PostLFRRequest;
using fulfil::dispense::commands::PostLFRResponse;
using fulfil::dispense::commands::DispenseResponse;


PostLFRRequest::PostLFRRequest(std::shared_ptr<std::string> command_id, std::shared_ptr<std::string> PrimaryKeyID,
                               std::shared_ptr<nlohmann::json> request_json)
{
  /**
   * The command id is still somewhat important here because
   * it is used when filtering out commands in the queue.
   */
  this->request_id = command_id;
  this->request_json = request_json;
  this->PrimaryKeyID = PrimaryKeyID;
}

std::shared_ptr<DispenseResponse> PostLFRRequest::execute()
{
  if(!this->delegate.expired())
  {
    Logger::Instance()->Debug("PostDispense PostLFRRequest execution now");
    std::shared_ptr<DispenseRequestDelegate> tmp_delegate = this->delegate.lock();

    std::shared_ptr<PostLFRResponse> response = tmp_delegate->handle_post_LFR(this->PrimaryKeyID,
                                                                                    this->request_id,
                                                                                    this->request_json);
    return response;
  }
  else
  {
    std::cout << "PostDispense Command Delegate Expired" << std::endl;
    return std::make_shared<PostLFRResponse>(this->request_id, 9); //TODO: change error code here
  }
}