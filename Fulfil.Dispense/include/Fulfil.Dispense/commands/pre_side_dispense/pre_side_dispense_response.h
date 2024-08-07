//
// Created by Jess on 8/6/24.
//

#ifndef FULFIL_COMPUTERVISION_PRE_SIDE_DISPENSE_RESPONSE_H
#define FULFIL_COMPUTERVISION_PRE_SIDE_DISPENSE_RESPONSE_H

#include <Fulfil.Dispense/commands/dispense_response.h>
#include "Fulfil.Dispense/dispense/side_dispense_error_codes.h"

using fulfil::dispense::side_dispense_error_codes::SideDispenseErrorCodes;

namespace fulfil::dispense::commands {
    class PreSideDispenseResponse final : public fulfil::dispense::commands::DispenseResponse
    {
    private:
        // fields set in the constructor
        std::shared_ptr<std::string> request_id;
        SideDispenseErrorCodes success_code{SideDispenseErrorCodes::Success};
        std::string error_description{""};

        // fields that are generated outside of the constructor
        std::shared_ptr<std::string> payload;

        // methods
        void encode_payload();

    public:
        explicit PreSideDispenseResponse(std::shared_ptr<std::string> request_id, 
                                         SideDispenseErrorCodes success_code, 
                                         std::string error_description=std::string(""));

        int dispense_payload_size() override;
        std::shared_ptr<std::string> get_command_id() override;
        std::shared_ptr<std::string> dispense_payload() override;
    };
} // namespace fulfil::dispense::commands

#endif //FULFIL_COMPUTERVISION_PRE_SIDE_DISPENSE_RESPONSE_H