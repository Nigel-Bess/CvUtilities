//
// Created by priyanka on 10/27/24.
//

#include <memory>
#include "commands/tcs_error_codes.h"


using fulfil::dispense::commands::TCSError;
using fulfil::dispense::commands::TCSErrorCodes;

// note: message param is optional and is default empty
TCSError::TCSError(TCSErrorCodes status_code, const std::string& description) {
    this->status_code = status_code;
    this->status_name = get_error_name_from_code(status_code);
    this->message = "TCS Algorithm error " + std::to_string(this->status_code) + ": " + this->status_name;
    this->description = description;
    // if the message parameter is non-empty, append
    if (!this->description.empty()) { this->message = this->message + " - " + this->description; }
}

const char* TCSError::what() const noexcept { return this->message.data(); }