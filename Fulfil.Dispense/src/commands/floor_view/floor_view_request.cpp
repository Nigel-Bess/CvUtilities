//
// Created by jessv on 5/24/24.
//

#include <Fulfil.Dispense/commands/floor_view/floor_view_request.h>
#include <Fulfil.Dispense/commands/dispense_request_delegate.h>
#include <Fulfil.Dispense/commands/floor_view/floor_view_response.h>


using fulfil::dispense::commands::DispenseRequestDelegate;
using fulfil::dispense::commands::FloorViewRequest;

FloorViewRequest::FloorViewRequest(std::shared_ptr<std::string> command_id,
                                                 std::shared_ptr<std::string> PrimaryKeyID,
                                                 std::shared_ptr<nlohmann::json> request_json)
{
    this->command_id = command_id;
    this->PrimaryKeyID = PrimaryKeyID;
    this->request_json = request_json;
}

std::shared_ptr<fulfil::dispense::commands::DispenseResponse> FloorViewRequest::execute()
{
    if(!this->delegate.expired())
    {
        std::shared_ptr<DispenseRequestDelegate> tmp_delegate = this->delegate.lock();
        std::shared_ptr<fulfil::dispense::commands::FloorViewResponse> floor_view_response =
                tmp_delegate->handle_floor_view(this->PrimaryKeyID, this->command_id, this->request_json); // calling method in dispense_manager
        return floor_view_response;
    }
    else
    {
        std::cout << "FloorViewDistance Command Delegate Expired" << std::endl;
        return std::make_shared<FloorViewResponse>(this->command_id,
                                                   1,
                                                   "FloorViewDistance Command Delegate Expired",
                                                   false,
                                                   false,
                                                   0); //Todo: change the error code used here if needed
    }
}
