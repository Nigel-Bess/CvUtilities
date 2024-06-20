//
// Created by amber on 6/20/24.
//

#include <Fulfil.CPPUtils/logging.h>
#include "Fulfil.Dispense/commands/side_dispense_target/side_dispense_target_request.h"
#include "Fulfil.Dispense/commands/side_dispense_target/side_dispense_target_response.h"


using fulfil::utils::Logger;
using fulfil::dispense::commands::SideDispenseTargetResponse;
using fulfil::dispense::commands::DispenseResponse;


SideDispenseTargetRequest::SideDispenseTargetRequest(std::shared_ptr<std::string> command_id, std::shared_ptr<std::string> PrimaryKeyID,
                                                 std::shared_ptr<nlohmann::json> request_json)
{
    /**
     * The command id is still somewhat important here because
     * it is used when filtering out commands in the queue.
     */
    this->command_id = command_id;
    this->request_json = request_json;
    this->PrimaryKeyID = PrimaryKeyID;
}

std::shared_ptr<DispenseResponse> SideDispenseTargetRequest::execute()
{
    if(!this->delegate.expired())
    {
        Logger::Instance()->Debug("SideDispenseTargetRequest execution now");
        std::shared_ptr<DispenseRequestDelegate> tmp_delegate = this->delegate.lock();

        std::shared_ptr<SideDispenseTargetResponse> response = tmp_delegate->handle_side_dispense_target(this->PrimaryKeyID,
                                                                                                     this->command_id,
                                                                                                     this->request_json);
        return response;
    }
    else
    {
        std::cout << "SideDispenseTarget Command Delegate Expired" << std::endl;
        return std::make_shared<SideDispenseTargetResponse>(this->command_id, 9); //TODO: change error code here
    }
}