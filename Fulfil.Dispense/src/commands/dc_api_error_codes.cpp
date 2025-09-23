//
// Created by jessv on 12/13/24.
//

#include <memory>
#include <string>
#include <Fulfil.CPPUtils/commands/dc_api_error_codes.h>

using fulfil::utils::commands::dc_api_error_codes::DcApiErrorCode;
using fulfil::utils::commands::dc_api_error_codes::DcApiError;
using fulfil::utils::commands::dc_api_error_codes::get_error_name_from_code;

// note: message param is optional and is default empty
DcApiError::DcApiError(DcApiErrorCode status_code,
                        const std::string &description) {
    this->status_code = status_code;
    this->status_name = get_error_name_from_code(status_code);
    this->message = "DC API error " + std::to_string(this->status_code) + ": " + this->status_name;
    this->description = description;
    // if the message parameter is non-empty, append
    if (!this->description.empty()) { this->message = this->message + " - " + this->description; }
}

const char* DcApiError::what() const noexcept { return this->message.data(); }

DcApiErrorCode DcApiError::get_status_code() const {
    return this->status_code;
}

std::string DcApiError::get_description() const {
    return this->description;
}