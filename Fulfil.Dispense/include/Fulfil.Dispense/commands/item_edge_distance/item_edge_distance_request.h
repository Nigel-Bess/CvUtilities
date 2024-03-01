//
// Created by amber on 10/12/20.
//

#ifndef FULFIL_DISPENSE_ITEM_EDGE_DISTANCE_REQUEST_H
#define FULFIL_DISPENSE_ITEM_EDGE_DISTANCE_REQUEST_H
#include <Fulfil.Dispense/commands/dispense_request.h>
#include <Fulfil.Dispense/commands/dispense_response.h>

namespace fulfil
{
    namespace dispense {
        namespace commands
        {
            class ItemEdgeDistanceRequest : public fulfil::dispense::commands::DispenseRequest
            {
            private:
                std::shared_ptr<nlohmann::json> request_json;
            public:
                ItemEdgeDistanceRequest(std::shared_ptr<std::string> command_id,
                                        std::shared_ptr<std::string> PrimaryKeyID,
                                        std::shared_ptr<nlohmann::json> request_json);
                /**
                 * Executes the pre drop request by calling the delegate and providing
                 * the details for the pre drop request.
                 * @return the dispense response from executing the pre drop request.
                 */
                std::shared_ptr<fulfil::dispense::commands::DispenseResponse> execute() override;
            };

        } // namespace commands
    } // namespace dispense
} // namespace fulfil
#endif //FULFIL_DISPENSE_ITEM_EDGE_DISTANCE_REQUEST_H
