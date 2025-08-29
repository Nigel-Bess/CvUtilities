//
// Created by Jess vdV on 7/2/25.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_COMPUTERVISION_PRE_PICKUP_CLIP_ACTUATOR_RESPONSE_H
#define FULFIL_COMPUTERVISION_PRE_PICKUP_CLIP_ACTUATOR_RESPONSE_H

#include <Fulfil.CPPUtils/commands/dc_api_error_codes.h>
#include <Fulfil.Dispense/commands/dispense_response.h>

using fulfil::utils::commands::dc_api_error_codes::DcApiErrorCode;

namespace fulfil::dispense::commands {
    class PrePickupClipActuatorResponse final : public fulfil::dispense::commands::DispenseResponse
    {
    private:
        std::shared_ptr<std::string> request_id;
        std::shared_ptr<std::string> payload;
        void encode_payload();

    public:
        explicit PrePickupClipActuatorResponse(std::shared_ptr<std::string> request_id, 
                                               std::shared_ptr<std::string> primary_key_id,
                                               int tote_id,
                                               int facility_id,
                                               std::shared_ptr<std::vector<bool>> clip_open_states,
                                               DcApiErrorCode success_code, 
                                               std::string error_description=std::string(""));

        int dispense_payload_size() override;
        std::shared_ptr<std::string> get_command_id() override;
        std::shared_ptr<std::string> dispense_payload() override;
        std::shared_ptr<std::string> primary_key_id;
        int tote_id{-1};
        int facility_id{-1};
        std::shared_ptr<std::vector<bool>> clip_open_states{nullptr};
        DcApiErrorCode success_code{DcApiErrorCode::Success};
        std::string error_description{""};
    };
} // namespace fulfil::dispense::commands

#endif //FULFIL_COMPUTERVISION_PRE_PICKUP_CLIP_ACTUATOR_RESPONSE_H