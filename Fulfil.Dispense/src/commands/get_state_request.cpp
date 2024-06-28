//
// Created by steve on 12/31/20.
//

#include "Fulfil.Dispense/commands/get_state_request.h"
#include <Fulfil.Dispense/commands/code_response.h>
#include <Fulfil.Dispense/commands/content_response.h>

using fulfil::dispense::commands::GetStateRequest;
using fulfil::dispense::commands::CodeResponse;
using fulfil::dispense::commands::DispenseResponse;


GetStateRequest::GetStateRequest(std::shared_ptr<std::string> command_id, std::shared_ptr<std::string> PrimaryKeyID,
                                 std::shared_ptr<nlohmann::json> request_json)
{
  this->request_id = command_id;
  this->request_json = request_json;
  this->PrimaryKeyID = PrimaryKeyID;
}

std::shared_ptr<DispenseResponse> GetStateRequest::execute()
{
  if(!this->delegate.expired())
  {
    std::shared_ptr<DispenseRequestDelegate> tmp_delegate = this->delegate.lock();

    auto error_code = tmp_delegate->handle_get_state(this->PrimaryKeyID, this->request_json);
    return std::make_shared<ContentResponse>(this->request_id, std::make_shared<std::string>(error_code), DepthCameras::MessageType::MESSAGE_TYPE_BAG_STATE_REQUEST);
    
  }
  else
  {
    std::cout << "GetState Command Delegate Expired" << std::endl;
    return std::make_shared<CodeResponse>(this->request_id, 9); //Todo: change the error code used here if needed
  }
}
