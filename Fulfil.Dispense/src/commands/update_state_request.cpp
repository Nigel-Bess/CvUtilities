//
// Created by steve on 12/31/20.
//

#include "Fulfil.Dispense/commands/update_state_request.h"
#include <Fulfil.Dispense/commands/code_response.h>
#include <Fulfil.Dispense/commands/content_response.h>
#include <Fulfil.CPPUtils/logging.h>

using fulfil::dispense::commands::UpdateStateRequest;
using fulfil::dispense::commands::CodeResponse;
using fulfil::dispense::commands::ContentResponse;
using fulfil::dispense::commands::DispenseResponse;
using fulfil::utils::Logger;


UpdateStateRequest::UpdateStateRequest(std::shared_ptr<std::string> command_id,
                                       std::shared_ptr<std::string> PrimaryKeyID,
                                       std::shared_ptr<nlohmann::json> request_json)
{
  this->request_id = command_id;
  this->request_json = request_json;
  this->PrimaryKeyID = PrimaryKeyID;
}

std::shared_ptr<DispenseResponse> UpdateStateRequest::execute()
{
  if(!this->delegate.expired())
  {
    std::shared_ptr<DispenseRequestDelegate> tmp_delegate = this->delegate.lock();

    auto error_code = tmp_delegate->handle_update_state(this->PrimaryKeyID, this->request_json);
    return std::make_shared<CodeResponse>(this->request_id, error_code);
  }
  else
  {
    Logger::Instance()->Error("Command Delegate Expired; Vars: HomeMotorRequest");
    return std::make_shared<CodeResponse>(this->request_id, 9); //Todo: change the error code used here if needed
  }
}
