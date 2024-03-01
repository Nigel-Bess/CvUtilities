//
// Created by amber on 10/12/20.
//

#ifndef FULFIL_DISPENSE_ITEM_EDGE_DISTANCE_RESPONSE_H
#define FULFIL_DISPENSE_ITEM_EDGE_DISTANCE_RESPONSE_H
#include <Fulfil.Dispense/commands/dispense_response.h>
#include <Fulfil.Dispense/tray/tray_result.h>

namespace fulfil::dispense::commands
        {
/**
 * The purpose of this class is to outline and contain
 * all of the information required for the response to
 * a pre drop response
 */
            class ItemEdgeDistanceResponse final : public fulfil::dispense::commands::DispenseResponse
            {
            private:

                /**
                 * The id for the request.
                 */
                std::shared_ptr<std::string> command_id;
                /**
                 *  success_code = 0 if successful, > 0 if there was an error
                 */
                int success_code;

                /**
                 * The payload to be sent in response to the request
                 */
                std::shared_ptr<std::string> payload;

                std::shared_ptr<fulfil::dispense::tray::TrayResult> tray_result;
                /**
                 * Encodes the payload in the payload string variable on this object.
                 */
                void encode_payload();
            public:

                explicit ItemEdgeDistanceResponse(std::shared_ptr<std::string> command_id, int success_code);

                ItemEdgeDistanceResponse(std::shared_ptr<std::string> command_id, std::shared_ptr<fulfil::dispense::tray::TrayResult> tray_result);

                std::shared_ptr<std::string> get_command_id() override;

                int dispense_payload_size() override;

                std::shared_ptr<std::string> dispense_payload() override;
            };
        } // namespace fulfil
#endif //FULFIL_DISPENSE_ITEM_EDGE_DISTANCE_RESPONSE_H
