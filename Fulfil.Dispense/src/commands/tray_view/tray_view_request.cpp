//
// Created by jessv on 2/3/25.
//

#include <Fulfil.Dispense/commands/tray_view/tray_view_request.h>
#include <Fulfil.Dispense/commands/dispense_request_delegate.h>
#include <Fulfil.Dispense/commands/tray_view/tray_view_response.h>


using fulfil::dispense::commands::DispenseRequestDelegate;
using fulfil::dispense::commands::TrayViewRequest;

TrayViewRequest::TrayViewRequest(std::shared_ptr<std::string> request_id,
                                                 std::shared_ptr<std::string> PrimaryKeyID,
                                                 std::shared_ptr<nlohmann::json> request_json)
{
    this->request_id = request_id;
    this->PrimaryKeyID = PrimaryKeyID;
    this->request_json = request_json;
}

std::shared_ptr<fulfil::dispense::commands::DispenseResponse> TrayViewRequest::execute()
{
    if(!this->delegate.expired())
    {
        std::shared_ptr<DispenseRequestDelegate> tmp_delegate = this->delegate.lock();
        std::shared_ptr<fulfil::dispense::commands::TrayViewResponse> tray_view_response =
                tmp_delegate->handle_tray_view(this->PrimaryKeyID, this->request_id, this->request_json); // calling method in dispense_manager
        return tray_view_response;
    }
    else
    {
        std::cout << "TrayView Command Delegate Expired" << std::endl;
        return std::make_shared<TrayViewResponse>(this->request_id, 256, std::make_shared<std::string>("TrayView Command Delegate Expired")); //Todo: change the error code used here if needed
    }
}
