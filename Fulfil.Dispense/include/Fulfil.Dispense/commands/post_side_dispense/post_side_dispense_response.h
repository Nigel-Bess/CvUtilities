//
// Created by amber on 6/20/24.
//

#ifndef FULFIL_COMPUTERVISION_POST_SIDE_DISPENSE_RESPONSE_H
#define FULFIL_COMPUTERVISION_POST_SIDE_DISPENSE_RESPONSE_H

#include <Fulfil.CPPUtils/commands/dc_api_error_codes.h>
#include <Fulfil.Dispense/commands/dispense_response.h>

using fulfil::utils::commands::dc_api_error_codes::DcApiErrorCode;

namespace fulfil::dispense::commands {
    class PostSideDispenseResponse final : public fulfil::dispense::commands::DispenseResponse
    {
    private:
        std::shared_ptr<std::string> payload;
        void encode_payload();
    public:
        std::shared_ptr<std::string> request_id;
        std::shared_ptr<std::string> primary_key_id;
        std::shared_ptr<std::vector<std::shared_ptr<std::vector<float>>>> occupancy_map;
        float square_width{-1};
        float square_height{-1};
        int items_dispensed{1};
        int bag_full_percent{0};
        int item_on_target_percent{0};
        std::vector<int> products_to_overflow{};
        DcApiErrorCode success_code{DcApiErrorCode::Success};
        std::string error_description{};

        explicit PostSideDispenseResponse(std::shared_ptr<std::string> request_id,
            std::shared_ptr<std::string> primary_key_id,
            std::shared_ptr<std::vector<std::shared_ptr<std::vector<float>>>> occupancy_map,
            float square_width,
            float square_height,
            DcApiErrorCode success_code,
            std::string error_description,
            int items_dispensed);
        int dispense_payload_size() override;
        std::shared_ptr<std::string> get_command_id() override;
        std::shared_ptr<std::string> dispense_payload() override;
    };
} // namespace fulfil::dispense::commands

#endif //FULFIL_COMPUTERVISION_POST_SIDE_DISPENSE_RESPONSE_H
