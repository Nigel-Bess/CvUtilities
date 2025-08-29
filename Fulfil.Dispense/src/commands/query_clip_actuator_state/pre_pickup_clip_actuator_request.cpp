#include <Fulfil.CPPUtils/logging.h>
#include "Fulfil.Dispense/commands/pre_pickup_clip_actuator/pre_pickup_clip_actuator_request.h"
#include <Fulfil.Dispense/commands/pre_pickup_clip_actuator/pre_pickup_clip_actuator_response.h>
#include <utility>

using fulfil::utils::Logger;
using fulfil::dispense::commands::PrePickupClipActuatorRequest;
using fulfil::dispense::commands::PrePickupClipActuatorResponse;
using fulfil::dispense::commands::DispenseResponse;

PrePickupClipActuatorRequest::PrePickupClipActuatorRequest(std::shared_ptr<std::string> command_id, std::shared_ptr<std::string> PrimaryKeyID,
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


std::shared_ptr<DispenseResponse> PrePickupClipActuatorRequest::execute()
{
    //if(!this->delegate.expired())
    //{
        Logger::Instance()->Debug("PrePickupClipActuatorRequest execution now");
        std::shared_ptr<DispenseRequestDelegate> tmp_delegate = this->delegate.lock();

        std::shared_ptr<PrePickupClipActuatorResponse> response = tmp_delegate->handle_pre_pickup_clip_actuator(
                                                                                  this->request_id,
                                                                                  this->request_json);
        return response;
    /*}
    else
    {
        std::cout << "PrePickupClipActuatorRequest Command Delegate Expired" << std::endl;
        return std::make_shared<PrePickupClipActuatorResponse>(this->request_id,
                                                         this->PrimaryKeyID,
                                                         this->request_json);
    }*/
}
