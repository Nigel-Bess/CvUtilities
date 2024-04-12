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
                int success_code {0};

                /**
                 * The payload to be sent in response to the request
                 */
                std::shared_ptr<std::string> payload;


                results_to_vlsg::TrayValidationCounts count_result{};
                results_to_vlsg::LaneItemDistance lane_distance_info{};
                /**
                 * Encodes the payload in the payload string variable on this object.
                 */
                void encode_payload();
            public:

                explicit ItemEdgeDistanceResponse(std::shared_ptr<std::string> command_id, int success_code);

                ItemEdgeDistanceResponse(std::vector<tray_count_api_comms::LaneCenterLine> transformed_pixel_centers,
                                         int fed_result, int cv_detected_item_length,
                                         results_to_vlsg::TrayValidationCounts lane_count_result,
                                         std::shared_ptr<std::string> command_id, int success_code = 0);

                ItemEdgeDistanceResponse(results_to_vlsg::LaneItemDistance lane_item_distance,
                                         results_to_vlsg::TrayValidationCounts lane_count_result,
                                         std::shared_ptr<std::string> command_id);

                std::shared_ptr<std::string> get_command_id() override;

                int dispense_payload_size() override;

                std::shared_ptr<std::string> dispense_payload() override;

                [[nodiscard]] int get_fed_value() const;
            };
        } // namespace fulfil
#endif //FULFIL_DISPENSE_ITEM_EDGE_DISTANCE_RESPONSE_H
