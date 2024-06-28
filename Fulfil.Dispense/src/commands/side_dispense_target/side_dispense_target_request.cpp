//
// Created by amber on 6/20/24.
//

#include <Fulfil.CPPUtils/logging.h>

#include <utility>
#include "Fulfil.Dispense/commands/side_dispense_target/side_dispense_target_request.h"
#include "Fulfil.Dispense/commands/side_dispense_target/side_dispense_target_response.h"


using fulfil::utils::Logger;
using fulfil::dispense::commands::SideDispenseTargetRequest;
using fulfil::dispense::commands::SideDispenseTargetResponse;

using fulfil::dispense::commands::DispenseResponse;


SideDispenseTargetRequest::SideDispenseTargetRequest(std::shared_ptr<std::string> request_id, std::shared_ptr<std::string> PrimaryKeyID,
                                                     std::shared_ptr<nlohmann::json> request_json)
{
    this->request_id = std::move(request_id);
    this->request_json = std::move(request_json);
    this->PrimaryKeyID = std::move(PrimaryKeyID);
}

std::shared_ptr<DispenseResponse> SideDispenseTargetRequest::execute()
{
    if(!this->delegate.expired())
    {
        Logger::Instance()->Debug("SideDispenseTargetRequest execution now");
        std::shared_ptr<DispenseRequestDelegate> tmp_delegate = this->delegate.lock();

        std::shared_ptr<SideDispenseTargetResponse> response = tmp_delegate->handle_side_dispense_target(
                                                                                                     this->request_id,
                                                                                                     this->request_json);
        return response;
    }
    else
    {
        std::cout << "SideDispenseTarget Command Delegate Expired" << std::endl;
        return std::make_shared<SideDispenseTargetResponse>(this->request_id); //TODO: change error code here
    }
}