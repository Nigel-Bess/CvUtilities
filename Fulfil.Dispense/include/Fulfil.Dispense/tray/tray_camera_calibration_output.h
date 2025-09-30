#pragma once

#include <chrono>
#include <array>

#include <Fulfil.CPPUtils/commands/dc_api_error_codes.h>

using fulfil::utils::commands::dc_api_error_codes::DcApiErrorCode;

namespace fulfil::dispense::tray
{
    struct TrayCameraCalibrationOutput
    {
        static constexpr uint8_t num_markers = 16;

        struct Point {
            float x{};
            float y{};
            float z{};
        };

        struct Frame {
            std::string unit;                           //pixel or meters
            std::array<Point, num_markers> points{};    //for the 16 markers
        };

        std::string camera_serial;
        std::string machine_name;
        std::string position;
        std::chrono::system_clock::time_point updated_at{};

        Frame pixel_coordinates{"pixel"};
        Frame camera_depth_coordinates{"meters"};
        Frame tray_depth_coordinates{"meters"};

        DcApiErrorCode return_code{DcApiErrorCode::Success};
        std::string error_description{""};
    };
}
