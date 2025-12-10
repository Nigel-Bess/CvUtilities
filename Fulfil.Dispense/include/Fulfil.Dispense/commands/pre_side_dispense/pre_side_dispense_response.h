//
// Created by Jess on 8/6/24.
//

#ifndef FULFIL_COMPUTERVISION_PRE_SIDE_DISPENSE_RESPONSE_H
#define FULFIL_COMPUTERVISION_PRE_SIDE_DISPENSE_RESPONSE_H

#include <Fulfil.CPPUtils/commands/dc_api_error_codes.h>
#include <Fulfil.Dispense/commands/dispense_response.h>
#include <json.hpp>
#include <Fulfil.CPPUtils/eigen.h>

using fulfil::utils::commands::dc_api_error_codes::DcApiErrorCode;

namespace fulfil::dispense::commands {
    class PreSideDispenseResponse final : public fulfil::dispense::commands::DispenseResponse
    {
    private:
        // fields set in the constructor
        std::shared_ptr<std::string> request_id;

        // fields that are set outside of the constructor
        std::shared_ptr<std::string> payload;

        // methods
        void encode_payload();

    public:
        explicit PreSideDispenseResponse(std::shared_ptr<std::string> request_id, 
                                         std::shared_ptr<std::string> primary_key_id,
                                         std::shared_ptr<std::vector<std::shared_ptr<std::vector<float>>>> occupancy_map,
                                         std::shared_ptr<nlohmann::json> occupancy_data,
                                         std::shared_ptr<std::vector<Eigen::Vector3d>> local_point_cloud_inside_cavity,
                                         float square_width,
                                         float square_height,
                                         DcApiErrorCode success_code, 
                                         std::string error_description=std::string(""));

        int dispense_payload_size() override;
        std::shared_ptr<std::string> get_command_id() override;
        std::shared_ptr<std::string> dispense_payload() override;
        std::shared_ptr<std::string> primary_key_id;
        std::shared_ptr<std::vector<std::shared_ptr<std::vector<float>>>> occupancy_map;
        std::shared_ptr<nlohmann::json> occupancy_data;
        std::shared_ptr<std::vector<Eigen::Vector3d>> local_point_cloud_inside_cavity;
        float square_width;
        float square_height;
        DcApiErrorCode success_code{DcApiErrorCode::Success};
        std::string error_description{""};
    };
} // namespace fulfil::dispense::commands

#endif //FULFIL_COMPUTERVISION_PRE_SIDE_DISPENSE_RESPONSE_H