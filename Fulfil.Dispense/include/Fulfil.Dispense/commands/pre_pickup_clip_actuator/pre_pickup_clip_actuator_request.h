//
// Created by Jess vdV on 7/2/25.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_COMPUTERVISION_PRE_PICKUP_CLIP_ACTUATOR_REQUEST_H
#define FULFIL_COMPUTERVISION_PRE_PICKUP_CLIP_ACTUATOR_REQUEST_H

#include <Fulfil.Dispense/commands/dispense_request.h>
#include <Fulfil.Dispense/commands/dispense_response.h>

namespace fulfil::dispense::commands {
    class PrePickupClipActuatorRequest final: public fulfil::dispense::commands::DispenseRequest
    {
    public:
        // tote id, facility id, bagcavityindex, expectedbagtype, beforeclipactuationattempt

        std::shared_ptr<nlohmann::json> request_json;

        explicit PrePickupClipActuatorRequest(std::shared_ptr<std::string> command_id, 
                                              std::shared_ptr<std::string> PrimaryKeyID,
                                              std::shared_ptr<nlohmann::json> request_json);

        std::shared_ptr<fulfil::dispense::commands::DispenseResponse> execute() override;
    };
} // namespace commands
#endif //FULFIL_COMPUTERVISION_PRE_PICKUP_CLIP_ACTUATOR_REQUEST_H

