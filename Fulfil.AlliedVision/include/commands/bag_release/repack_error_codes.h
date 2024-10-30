//
// Created by priyanka on 10/27/24.
//

#ifndef FULFIL_DISPENSE_REPACK_ERROR_CODES_H
#define FULFIL_DISPENSE_REPACK_ERROR_CODES_H

#include <memory>

namespace fulfil::dispense::commands {

    /* error codes for Repack CV Empty Bag Algorithm*/
    enum RepackErrorCodes {
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
    static std::string get_error_name_from_code(RepackErrorCodes status_code) {
        switch (status_code) {
        case RepackErrorCodes::Success:
            return "Success";
        case RepackErrorCodes::NoMarkersDetected:
            return "NoMarkersDetected";
        case RepackErrorCodes::NotEnoughMarkersDetected:
            return "NotEnoughMarkersDetected";
        case RepackErrorCodes::VimbaCameraError:
            return "VimbaCameraError";
        case RepackErrorCodes::UnspecifiedError:
            return "UnspecifiedError";
        default:
            return "UndefinedError - Not in RepackErrorCodes";
        }
    }

    /**
     * RepackError is a custom exception for any Fulfil-defined RepackErrorCodes to be used in the
     * in CV Empty Bag Algorithm
     */
    class RepackError : public std::exception {
        /* status code of the error */
        RepackErrorCodes status_code;
        /* name corresponding to the status code */
        std::string status_name;
        /* Longer message of form "Repack CV Empty Bag Algorithm error {STATUS CODE}: {ERROR NAME} - {this->description} "*/
        std::string message;
        /* default is empty string, and will be appended to the message field */
        std::string description;
    public:
        /**
         * RepackError exception to be thrown in the repack empty bag algorithm
         * @param status_code denoting which error in the RepackErrorCodes enum is active
         * and will be returned in the BagReleaseResponse
         */
        explicit RepackError(RepackErrorCodes status_code, const std::string& description = "");

        [[nodiscard]] const char* what() const noexcept override;
    };
} // namespace fulfil::dispense::commands
#endif //FULFIL_DISPENSE_REPACK_ERROR_CODES_H