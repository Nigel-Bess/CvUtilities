//
// Created by Jess on 8/6/24.
//

#ifndef FULFIL_COMPUTERVISION_PRE_SIDE_DISPENSE_REQUEST_H
#define FULFIL_COMPUTERVISION_PRE_SIDE_DISPENSE_REQUEST_H

#include <Fulfil.Dispense/commands/dispense_request.h>
#include <Fulfil.Dispense/commands/dispense_response.h>

namespace fulfil::dispense::commands {
    class PreSideDispenseRequest final: public fulfil::dispense::commands::DispenseRequest
    {
    public:

        std::shared_ptr<nlohmann::json> request_json;

        explicit PreSideDispenseRequest(std::shared_ptr<std::string> command_id, std::shared_ptr<std::string> PrimaryKeyID,
                                         std::shared_ptr<nlohmann::json> request_json);

        std::shared_ptr<fulfil::dispense::commands::DispenseResponse> execute() override;
    };
} // namespace commands
#endif //FULFIL_COMPUTERVISION_PRE_SIDE_DISPENSE_REQUEST_H
