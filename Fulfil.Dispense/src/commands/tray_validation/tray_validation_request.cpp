//
// Created by steve on 5/12/20.
//

#include <Fulfil.Dispense/commands/dispense_request_delegate.h>
#include <Fulfil.Dispense/commands/tray_validation/tray_validation_request.h>
#include <Fulfil.Dispense/commands/tray_validation/tray_validation_response.h>
#include <Fulfil.DepthCam/visualization.h>
#include <Fulfil.Dispense/commands/code_response.h>

using fulfil::dispense::commands::TrayValidationRequest;
using fulfil::dispense::commands::CodeResponse;
using fulfil::dispense::commands::DispenseRequestDelegate;

TrayValidationRequest::TrayValidationRequest(std::shared_ptr<std::string> command_id, std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json)
{
  this->request_id = command_id;
  this->PrimaryKeyID = PrimaryKeyID;
  this->request_json = request_json;
}

std::shared_ptr<fulfil::dispense::commands::DispenseResponse> fulfil::dispense::commands::TrayValidationRequest::execute()
{
  if(!this->delegate.expired())
  {
    std::shared_ptr<DispenseRequestDelegate> tmp_delegate = this->delegate.lock();
    return tmp_delegate->handle_tray_validation(this->request_id, this->request_json); //calls method in dispense_manager
  }
  else
  {
    std::cout << "Tray Height Command Delegate Expired" << std::endl;
    return std::make_shared<CodeResponse>(this->request_id, 9);
  }
}

