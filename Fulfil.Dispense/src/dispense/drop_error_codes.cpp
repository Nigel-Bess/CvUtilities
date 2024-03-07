//
// Created by jessv on 2/9/24.
//

#include <memory>
#include "Fulfil.Dispense/dispense/drop_error_codes.h"


using fulfil::dispense::drop_target_error_codes::DropTargetErrorCodes;
using fulfil::dispense::drop_target_error_codes::DropTargetError;
using fulfil::dispense::drop_target_error_codes::get_error_name_from_code;

// note: message param is optional and is default empty
DropTargetError::DropTargetError(DropTargetErrorCodes status_code,
                const std::string &description) {
    this->status_code = status_code;
    this->status_name = get_error_name_from_code(status_code);
    this->message = "Drop Target Algorithm error " + std::to_string(this->status_code) + ": " + this->status_name;
    this->description = description;
    // if the message parameter is non-empty, append
    if (!description.empty()) this->message + " - " + this->description;
}

const char* DropTargetError::what() const noexcept { return this->message.data(); }

DropTargetErrorCodes DropTargetError::get_status_code() {
    return this->status_code;
}

std::string DropTargetError::get_description() {
    return this->description;
}