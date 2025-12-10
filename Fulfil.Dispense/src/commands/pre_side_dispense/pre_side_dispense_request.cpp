#include <Fulfil.CPPUtils/logging.h>
#include "Fulfil.Dispense/commands/pre_side_dispense/pre_side_dispense_request.h"
#include <Fulfil.Dispense/commands/pre_side_dispense/pre_side_dispense_response.h>

#include <utility>


using fulfil::utils::Logger;
using fulfil::dispense::commands::PreSideDispenseRequest;
using fulfil::dispense::commands::PreSideDispenseResponse;
using fulfil::dispense::commands::DispenseResponse;


PreSideDispenseRequest::PreSideDispenseRequest(std::shared_ptr<std::string> command_id, 
                                               std::shared_ptr<std::string> PrimaryKeyID,
                                               std::shared_ptr<nlohmann::json> request_json)
{
    /**
     * The command id is still somewhat important here because
     * it is used when filtering out commands in the queue.
     */
    this->request_id = std::move(command_id);
    this->request_json = request_json;
    this->PrimaryKeyID = std::move(PrimaryKeyID);
}

std::shared_ptr<DispenseResponse> PreSideDispenseRequest::execute()
{
    if(!this->delegate.expired())
    {
        Logger::Instance()->Debug("PreSideDispenseRequest execution now");
        std::shared_ptr<DispenseRequestDelegate> tmp_delegate = this->delegate.lock();

        std::shared_ptr<PreSideDispenseResponse> response = tmp_delegate->handle_pre_side_dispense(
                                                                                  this->request_id,
                                                                                  this->request_json);
        return response;
    }
    else
    {
        std::cout << "PreSideDispense Command Delegate Expired" << std::endl;
        return std::make_shared<PreSideDispenseResponse>(this->request_id, this->PrimaryKeyID, nullptr, nullptr, nullptr, -1, -1, DcApiErrorCode::CommandDelegateExpired);
    }
}