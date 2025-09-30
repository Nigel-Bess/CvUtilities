#include "Fulfil.Dispense/commands/tray_camera_calibration/tray_camera_calibration_response.h"
#include <json.hpp>
#include <Fulfil.CPPUtils/logging.h>

using fulfil::dispense::commands::CalibrateTrayDepthCameraResponse;
using fulfil::utils::Logger;
using fulfil::utils::commands::dc_api_error_codes::DcApiErrorCode;

CalibrateTrayDepthCameraResponse::CalibrateTrayDepthCameraResponse(
    std::shared_ptr<std::string> req_id,
    std::shared_ptr<fulfil::dispense::tray::TrayCameraCalibrationOutput> calibration_output
) : request_id(req_id),
    tray_calibration_output(calibration_output)
{
}

int CalibrateTrayDepthCameraResponse::dispense_payload_size() {

    if (!this->payload) {
        this->encode_payload();
    }
    return this->payload->length() + 1;
}

std::shared_ptr<std::string> CalibrateTrayDepthCameraResponse::get_command_id() {

    return this->request_id;
}

std::shared_ptr<std::string> CalibrateTrayDepthCameraResponse::dispense_payload() {

    if (!(this->payload)) {
        this->encode_payload();
    }
    return this->payload;
}

void CalibrateTrayDepthCameraResponse::encode_payload() {
    nlohmann::json result_json{};
    
    if (this->tray_calibration_output) {
        auto& output = *this->tray_calibration_output;
        
        // Basic information
        result_json["Camera_Serial"] = output.camera_serial;
        result_json["Machine_Name"] = output.machine_name;
        result_json["Position"] = output.position;
        result_json["Error"] = static_cast<int>(output.return_code);
        result_json["Error_Description"] = output.error_description;
        
        // Convert timestamp to seconds since epoch
        auto epoch_time = std::chrono::duration_cast<std::chrono::seconds>(
            output.updated_at.time_since_epoch()).count();
        result_json["Updated_At"] = epoch_time;
        
        // Pixel coordinates
        nlohmann::json pixel_coords = nlohmann::json::array();
        for (size_t i = 0; i < output.pixel_coordinates.points.size(); ++i) {
            const auto& point = output.pixel_coordinates.points[i];
            nlohmann::json point_json;
            point_json["x"] = point.x;
            point_json["y"] = point.y;
            point_json["z"] = point.z;
            pixel_coords.push_back(point_json);
        }
        result_json["Pixel_Coordinates"] = {
            {"unit", output.pixel_coordinates.unit},
            {"points", pixel_coords}
        };
        
        // Camera depth coordinates
        nlohmann::json camera_coords = nlohmann::json::array();
        for (size_t i = 0; i < output.camera_depth_coordinates.points.size(); ++i) {
            const auto& point = output.camera_depth_coordinates.points[i];
            nlohmann::json point_json;
            point_json["x"] = point.x;
            point_json["y"] = point.y;
            point_json["z"] = point.z;
            camera_coords.push_back(point_json);
        }
        result_json["Camera_Depth_Coordinates"] = {
            {"unit", output.camera_depth_coordinates.unit},
            {"points", camera_coords}
        };
        
        // Tray depth coordinates
        nlohmann::json tray_coords = nlohmann::json::array();
        for (size_t i = 0; i < output.tray_depth_coordinates.points.size(); ++i) {
            const auto& point = output.tray_depth_coordinates.points[i];
            nlohmann::json point_json;
            point_json["x"] = point.x;
            point_json["y"] = point.y;
            point_json["z"] = point.z;
            tray_coords.push_back(point_json);
        }
        result_json["Tray_Depth_Coordinates"] = {
            {"unit", output.tray_depth_coordinates.unit},
            {"points", tray_coords}
        };
    } else {
        // Handle case where calibration output is null
        result_json["Error"] = static_cast<int>(DcApiErrorCode::UnspecifiedError);
        result_json["Error_Description"] = "Calibration output is null";
    }

    std::string json_string = result_json.dump();
    Logger::Instance()->Info("Encoding CalibrateTrayDepthCameraResponse as: {}", json_string);
    int json_length = json_string.size();
    const char *json_text = json_string.c_str();
    char *response = new char[json_length + 1];
    memcpy(response, json_text, json_length);
    response[json_length] = 0;
    this->payload = std::make_shared<std::string>(response);
    delete [] response;
}
