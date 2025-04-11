//
// Created by Jess on 12/13/24.
//

#ifndef FULFIL_DISPENSE_DC_API_ERROR_CODES_H
#define FULFIL_DISPENSE_DC_API_ERROR_CODES_H

#include <memory>

namespace fulfil::utils::commands::dc_api_error_codes {

    /* error codes must match what is received in FC */
    enum DcApiErrorCode {
        Success = 0,
        /* No Aruco markers detected */
        NoMarkersDetected = 1,
        /* Not enough Aruco markers detected detect enough Aruco markers */
        NotEnoughMarkersDetected = 2,
        /* Bot in unexpected position */
        UnexpectedBagPosition = 3,
        /* Invalid Request format or components */
        InvalidRequest = 4,
        /* Item dimensions are too large for the destination bag */
        ItemLargerThanBag = 5,
        /* Bag is too full for additional items */
        NoViableTarget_BagIsFull = 6,
        /* Bag is too full for this item */
        NoViableTarget_NoSpaceForItem = 7,
        /* Bag does not have a low-risk dispense impact point for this item */
        NoViableTarget_DamageRisk = 8,
        /* Bag was expected to be empty but appears to be non-empty */
        EmptyBagNotEmpty = 9,
        /* Unspecified error encountered in Depth Cam code */
        UnspecifiedError = 10,
        /* Algorithm Fail, Target Out of Bounds */
        AlgorithmFail_TargetOutOfBounds = 11,
        /* Error used to bypass certain parts of the depth cam algorithm */
        AlgorithmFail_Bypass = 12,
        /* Bag ID mismatch */
        BagIdMismatch = 13,
        /* Bag has items with high risk of damage, with no viable targets for more dispenses */
        NoViableTarget_DamageRisk_PickupRequired = 14,
        /* Bot rotation status could not be determined */
        CouldNotDetermineBotRotationStatus = 15,
        /* Incompatible item dimensions and bot limits, check item dimensions/orientation */
        NoViableTarget_ItemDimensionsIncompatibleWithBotLimits = 16,
        /* Invalid value for the distance the bot platform has left to move */
        InvalidRemainingPlatformValue = 17,
        /* Invalid bot side limits */
        InvalidBotSideLimitValue = 18,
        /* Invalid item material code, check item metadata */
        InvalidItemMaterialCode = 19,
        /* Invalid item mass, check item metadata */
        InvalidItemMass = 20,
        /* Invalid item dimensions, check item metadata */
        InvalidItemDimensions = 21,
        /* Recoverable RealSense Error */
        RecoverableRealSenseError = 22,
        /* Pre Image empty for pre-post compare */
        EmptyPreImage = 23,
        /* Post Image empty for pre-post compare */
        EmptyPostImage = 24,
        /* Frame refresh error */
        FrameRefreshError = 25,
        /* UnrecoverableRealSenseError */
        UnrecoverableRealSenseError = 255,
        /* CommandDelegateExpired */
        CommandDelegateExpired = 256
    };

    /**
     * Gets the name of the given status code
     */
    static std::string get_error_name_from_code(DcApiErrorCode status_code) {
        switch (status_code) {
            case DcApiErrorCode::Success:
                return "Success";
            case DcApiErrorCode::NoMarkersDetected:
                return "NoMarkersDetected";
            case DcApiErrorCode::NotEnoughMarkersDetected:
                return "NotEnoughMarkersDetected";
            case DcApiErrorCode::UnexpectedBagPosition:
                return "UnexpectedBagPosition, Not Centered";
            case DcApiErrorCode::InvalidRequest:
                return "InvalidRequest";
            case DcApiErrorCode::ItemLargerThanBag:
                return "ItemLargerThanBag";
            case DcApiErrorCode::NoViableTarget_BagIsFull:
                return "NoViableTarget_BagIsFull";
            case DcApiErrorCode::NoViableTarget_NoSpaceForItem:
                return "NoViableTarget_NoSpaceForItem";
            case DcApiErrorCode::NoViableTarget_DamageRisk:
                return "NoViableTarget_DamageRisk";
            case DcApiErrorCode::EmptyBagNotEmpty:
                return "EmptyBagNotEmpty";
            case DcApiErrorCode::UnspecifiedError:
                return "UnspecifiedError";
            case DcApiErrorCode::AlgorithmFail_TargetOutOfBounds:
                return "AlgorithmFail_TargetOutOfBounds";
            case DcApiErrorCode::AlgorithmFail_Bypass:
                return "AlgorithmFail_Bypass";
            case DcApiErrorCode::BagIdMismatch:
                return "BagIdMismatch";
            case DcApiErrorCode::NoViableTarget_DamageRisk_PickupRequired:
                return "NoViableTarget_DamageRisk_PickupRequired";
            case DcApiErrorCode::CouldNotDetermineBotRotationStatus:
                return "CouldNotDetermineBotRotationStatus";
            case DcApiErrorCode::NoViableTarget_ItemDimensionsIncompatibleWithBotLimits:
                return "NoViableTarget_ItemDimensionsIncompatibleWithBotLimits";
            case DcApiErrorCode::InvalidRemainingPlatformValue:
                return "InvalidRemainingPlatformValue";
            case DcApiErrorCode::InvalidBotSideLimitValue:
                return "InvalidBotSideLimitValue";
            case DcApiErrorCode::InvalidItemMaterialCode:
                return "InvalidItemMaterialCode";
            case DcApiErrorCode::InvalidItemMass:
                return "InvalidItemMass";
            case DcApiErrorCode::InvalidItemDimensions:
                return "InvalidItemDimensions";
            case DcApiErrorCode::RecoverableRealSenseError:
                return "RecoverableRealSenseError";
            case DcApiErrorCode::EmptyPreImage:
                return "EmptyPreImage";
            case DcApiErrorCode::EmptyPostImage:
                return "EmptyPostImage";
            case DcApiErrorCode::FrameRefreshError:
                return "FrameRefreshError";
            case DcApiErrorCode::UnrecoverableRealSenseError:
                return "UnrecoverableRealSenseError";
            case DcApiErrorCode::CommandDelegateExpired:
                return "CommandDelegateExpired";
            default:
                return "UndefinedError - Not in DcApiErrorCode";
        }
    }

    /**
     * DcApiError is a custom exception for any Fulfil-defined DcApiErrorCode to be used in the DC API. 
     */
    class DcApiError : public std::exception {
        /* status code of the error according to the Depth Cam API Communication Cookbook */
        DcApiErrorCode status_code;
        /* name corresponding to the status code */
        std::string status_name;
        /* Longer message of form "DC API error {STATUS CODE}: {ERROR NAME} - {this->description} "*/
        std::string message;
        /* default is empty string, and will be appended to the message field */
        std::string description;
    public:
        /**
         * DcApiErrorCode exception to be thrown in the DC API
         * @param status_code denoting which error in the DcApiErrorCode enum is active
         * @param description optional parameter to be APPENDED to default message, for adding more detail to the error
         * and will be returned in the response
         */
        explicit DcApiError(DcApiErrorCode status_code,
                            const std::string &description = "");

        [[nodiscard]] const char *what() const noexcept override;

        /**
        * Getter for the error's status code
        */
        DcApiErrorCode get_status_code();

        /**
        * Getter for the error's custom description
        */
        std::string get_description();
    };
} // namespace fulfil::utils::commands::dc_api_error_codes
#endif //FULFIL_DISPENSE_DC_API_ERROR_CODES_H
