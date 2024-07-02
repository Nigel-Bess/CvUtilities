//
// Created by amber on 6/20/24.
//

#include "Fulfil.Dispense/commands/pre_side_dispense_request.h"
#include <Fulfil.Dispense/commands/code_response.h>

#include <utility>

using fulfil::dispense::commands::PreSideDispenseRequest;
using fulfil::dispense::commands::CodeResponse;
using fulfil::dispense::commands::DispenseResponse;

PreSideDispenseRequest::PreSideDispenseRequest(std::shared_ptr<std::string> request_id, std::shared_ptr<std::string> PrimaryKeyID,
                                               std::shared_ptr<nlohmann::json> request_json)
{
    this->request_id = std::move(request_id);
    this->request_json = std::move(request_json);
    this->PrimaryKeyID = std::move(PrimaryKeyID);
}

std::shared_ptr<DispenseResponse> PreSideDispenseRequest::execute()
{
    if(!this->delegate.expired())
    {
        std::shared_ptr<DispenseRequestDelegate> tmp_delegate = this->delegate.lock();

        int error_code = tmp_delegate->handle_pre_side_dispense(this->PrimaryKeyID, this->request_json);

        return std::make_shared<CodeResponse>(this->request_id, error_code);
    }
    else
    {
        std::cout << "PreDropLFB Command Delegate Expired" << std::endl;
        return std::make_shared<CodeResponse>(this->request_id, 9); //Todo: change the error code used here if needed
    }
}
