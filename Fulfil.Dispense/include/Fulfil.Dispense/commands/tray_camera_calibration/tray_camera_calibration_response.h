#pragma once

#include <Fulfil.Dispense/commands/dispense_response.h>
#include <Fulfil.Dispense/tray/tray_camera_calibration_output.h>

namespace fulfil::dispense::commands {

    class CalibrateTrayDepthCameraResponse final : public fulfil::dispense::commands::DispenseResponse {

        std::shared_ptr<std::string> request_id;
        std::shared_ptr<std::string> payload;

        void encode_payload();

    public:
        explicit CalibrateTrayDepthCameraResponse(
            std::shared_ptr<std::string> req_id,
            std::shared_ptr<fulfil::dispense::tray::TrayCameraCalibrationOutput> calibration_output = nullptr
        );

        int dispense_payload_size() override;
        std::shared_ptr<std::string> get_command_id() override;
        std::shared_ptr<std::string> dispense_payload() override;


        std::shared_ptr<fulfil::dispense::tray::TrayCameraCalibrationOutput> tray_calibration_output;
    };
} // namespace fulfil::dispense::commands
