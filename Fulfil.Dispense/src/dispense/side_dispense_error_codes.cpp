//
// Created by jessv on 8/6/24.
//

#include <memory>
#include <string>
#include "Fulfil.Dispense/dispense/side_dispense_error_codes.h"

using fulfil::dispense::side_dispense_error_codes::SideDispenseErrorCodes;
using fulfil::dispense::side_dispense_error_codes::SideDispenseError;
using fulfil::dispense::side_dispense_error_codes::get_error_name_from_code;

// note: message param is optional and is default empty
SideDispenseError::SideDispenseError(SideDispenseErrorCodes status_code,
                const std::string &description) {
    this->status_code = status_code;
    this->status_name = get_error_name_from_code(status_code);
    this->message = "Side Dispense Algorithm error " + std::to_string(this->status_code) + ": " + this->status_name;
    this->description = description;
    // if the message parameter is non-empty, append
    if (!this->description.empty()) { this->message = this->message + " - " + this->description; }
}

const char* SideDispenseError::what() const noexcept { return this->message.data(); }

SideDispenseErrorCodes SideDispenseError::get_status_code() {
    return this->status_code;
}

std::string SideDispenseError::get_description() {
    return this->description;
}