//
// Created by amber on 6/20/24.
//

//
// Created by steve on 12/31/20.
//

#include "Fulfil.Dispense/commands/pre_side_dispense_request.h"
#include <Fulfil.Dispense/commands/code_response.h>

using fulfil::dispense::commands::PreSideDispenseRequest;
using fulfil::dispense::commands::CodeResponse;
using fulfil::dispense::commands::DispenseResponse;

PreSideDispenseRequest::PreSideDispenseRequest(std::shared_ptr<std::string> command_id, std::shared_ptr<std::string> PrimaryKeyID,
                             std::shared_ptr<nlohmann::json> request_json)
{
    this->command_id = command_id;
    this->request_json = request_json;
    this->PrimaryKeyID = PrimaryKeyID;
}

std::shared_ptr<DispenseResponse> PreSideDispenseRequest::execute()
{
    if(!this->delegate.expired())
    {
        std::shared_ptr<DispenseRequestDelegate> tmp_delegate = this->delegate.lock();

        int error_code = tmp_delegate->handle_pre_side_dispense(this->PrimaryKeyID, this->request_json);

        return std::make_shared<CodeResponse>(this->command_id, error_code);
    }
    else
    {
        std::cout << "PreDropLFB Command Delegate Expired" << std::endl;
        return std::make_shared<CodeResponse>(this->command_id, 9); //Todo: change the error code used here if needed
    }
}
