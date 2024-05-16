#include <Fulfil.CPPUtils/logging.h>
#include "Fulfil.Dispense/commands/post_side_dispense/post_side_dispense_request.h"
#include <Fulfil.Dispense/commands/post_side_dispense/post_side_dispense_response.h>


using fulfil::utils::Logger;
using fulfil::dispense::commands::PostSideDispenseRequest;
using fulfil::dispense::commands::PostSideDispenseResponse;
using fulfil::dispense::commands::DispenseResponse;


PostSideDispenseRequest::PostSideDispenseRequest(std::shared_ptr<std::string> command_id, std::shared_ptr<std::string> PrimaryKeyID,
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

std::shared_ptr<DispenseResponse> PostSideDispenseRequest::execute()
{
    if(!this->delegate.expired())
    {
        Logger::Instance()->Debug("PostDispense PostSideDispenseRequest execution now");
        std::shared_ptr<DispenseRequestDelegate> tmp_delegate = this->delegate.lock();

        std::shared_ptr<PostSideDispenseResponse> response = tmp_delegate->handle_post_side_dispense(
                                                                                  this->command_id,
                                                                                  this->request_json);
        return response;
    }
    else
    {
        std::cout << "PostDispense Command Delegate Expired" << std::endl;
        return std::make_shared<PostSideDispenseResponse>(this->command_id); //TODO: Must add error code back
    }
}