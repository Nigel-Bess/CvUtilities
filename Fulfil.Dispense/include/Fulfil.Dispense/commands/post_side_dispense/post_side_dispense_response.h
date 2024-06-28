//
// Created by amber on 6/20/24.
//

#ifndef FULFIL_COMPUTERVISION_POST_SIDE_DISPENSE_RESPONSE_H
#define FULFIL_COMPUTERVISION_POST_SIDE_DISPENSE_RESPONSE_H
#include <Fulfil.Dispense/commands/dispense_response.h>

namespace fulfil::dispense::commands {
    class PostSideDispenseResponse final : public fulfil::dispense::commands::DispenseResponse
    {
    private:

        std::shared_ptr<std::string> request_id;
        int items_dispensed{1};
        int bag_full_percent{0};
        int item_on_target_percent{0};
        std::vector<int> products_to_overflow{};
        int success_code{0};
        std::string error_description{};
        std::shared_ptr<std::string> payload;
        void encode_payload();
    public:

        explicit PostSideDispenseResponse(std::shared_ptr<std::string> request_handshake_id);
        int dispense_payload_size() override;
        std::shared_ptr<std::string> get_command_id() override;
        std::shared_ptr<std::string> dispense_payload() override;
    };
} // namespace fulfil::dispense::commands

#endif //FULFIL_COMPUTERVISION_POST_SIDE_DISPENSE_RESPONSE_H
