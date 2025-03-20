//
// Created by priyanka on 10/27/24.
//

#ifndef FULFIL_DISPENSE_TCS_ERROR_CODES_H
#define FULFIL_DISPENSE_TCS_ERROR_CODES_H

#include <memory>

namespace fulfil::dispense::commands::tcs {

    /* error codes for TCS CV Algorithms */
    enum TCSErrorCodes {
        Success = 0,
        /* No Aruco markers detected */
        NoMarkersDetected = 1,
        /* Not enough Aruco markers detected */
        NotEnoughMarkersDetected = 2,
        /* Unspecified error encountered in Empty Bag Algorithm */
        UnspecifiedError = 10,
        /* Vimba Camera Errors */
        VimbaCameraError = 11,
    };

    /**
     * Gets the name of the given status code
     */
    static std::string get_error_name_from_code(TCSErrorCodes status_code) {
        switch (status_code) {
        case TCSErrorCodes::Success:
            return "Success";
        case TCSErrorCodes::NoMarkersDetected:
            return "NoMarkersDetected";
        case TCSErrorCodes::NotEnoughMarkersDetected:
            return "NotEnoughMarkersDetected";
        case TCSErrorCodes::VimbaCameraError:
            return "VimbaCameraError";
        case TCSErrorCodes::UnspecifiedError:
            return "UnspecifiedError";
        default:
            return "UndefinedError - Not in TCSErrorCodes";
        }
    }

    /**
     * TCSError is a custom exception for any Fulfil-defined TCSErrorCodes to be used in the
     * in CV Empty Bag Algorithm
     */
    class TCSError : public std::exception {
        /* status code of the error */
        TCSErrorCodes status_code;
        /* name corresponding to the status code */
        std::string status_name;
        /* Longer message of form TCS CV Algorithm error {STATUS CODE}: {ERROR NAME} - {this->description} "*/
        std::string message;
        /* default is empty string, and will be appended to the message field */
        std::string description;
    public:
        /**
         * TCSError exception to be thrown in some TCS algorithm
         * @param status_code denoting which error in the TCSErrorCodes enum is active
         * and will be returned in the TCSResponse
         */
        explicit TCSError(TCSErrorCodes status_code, const std::string& description = "");

        [[nodiscard]] const char* what() const noexcept override;
    };
} // namespace fulfil::dispense::commands::tcs
#endif //FULFIL_DISPENSE_TCS_ERROR_CODES_H