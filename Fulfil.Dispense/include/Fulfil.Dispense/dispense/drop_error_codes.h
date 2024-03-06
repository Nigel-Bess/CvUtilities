//
// Created by jessv on 2/9/24.
//

#ifndef FULFIL_DISPENSE_DROP_ERROR_CODES_H
#define FULFIL_DISPENSE_DROP_ERROR_CODES_H

#include <memory>

namespace fulfil::dispense::drop_target_error_codes {

    /* error codes defined in Depth Cam API Communication Cookbook, must match what is received in FC */
    enum DropTargetErrorCodes {
        Success = 0,
        /* No Aruco markers detected */
        NoMarkersDetected = 1,
        /* Not enough Aruco markers detected detect enough Aruco markers */
        NotEnoughMarkersDetected = 2,
        /* Bot in unexpected position */
        UnexpectedBagPosition = 3,
        /* Invalid Drop Target Request format or components */
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
        /* UnrecoverableRealSenseError */
        UnrecoverableRealSenseError = 255,
        /* CommandDelegateExpired */
        CommandDelegateExpired = 256
    };

    /**
     * Gets the name of the given status code
     */
    static std::string get_error_name_from_code(DropTargetErrorCodes status_code) {
        switch (status_code) {
            case DropTargetErrorCodes::Success:
                return "Success";
            case DropTargetErrorCodes::NoMarkersDetected:
                return "NoMarkersDetected";
            case DropTargetErrorCodes::NotEnoughMarkersDetected:
                return "NotEnoughMarkersDetected";
            case DropTargetErrorCodes::UnexpectedBagPosition:
                return "UnexpectedBagPosition, Not Centered";
            case DropTargetErrorCodes::InvalidRequest:
                return "InvalidRequest";
            case DropTargetErrorCodes::ItemLargerThanBag:
                return "ItemLargerThanBag";
            case DropTargetErrorCodes::NoViableTarget_BagIsFull:
                return "NoViableTarget_BagIsFull";
            case DropTargetErrorCodes::NoViableTarget_NoSpaceForItem:
                return "NoViableTarget_NoSpaceForItem";
            case DropTargetErrorCodes::NoViableTarget_DamageRisk:
                return "NoViableTarget_DamageRisk";
            case DropTargetErrorCodes::EmptyBagNotEmpty:
                return "EmptyBagNotEmpty";
            case DropTargetErrorCodes::UnspecifiedError:
                return "UnspecifiedError";
            case DropTargetErrorCodes::AlgorithmFail_TargetOutOfBounds:
                return "AlgorithmFail_TargetOutOfBounds";
            case DropTargetErrorCodes::AlgorithmFail_Bypass:
                return "AlgorithmFail_Bypass";
            case DropTargetErrorCodes::BagIdMismatch:
                return "BagIdMismatch";
            case DropTargetErrorCodes::NoViableTarget_DamageRisk_PickupRequired:
                return "NoViableTarget_DamageRisk_PickupRequired";
            case DropTargetErrorCodes::CouldNotDetermineBotRotationStatus:
                return "CouldNotDetermineBotRotationStatus";
            case DropTargetErrorCodes::NoViableTarget_ItemDimensionsIncompatibleWithBotLimits:
                return "NoViableTarget_ItemDimensionsIncompatibleWithBotLimits";
            case DropTargetErrorCodes::InvalidRemainingPlatformValue:
                return "InvalidRemainingPlatformValue";
            case DropTargetErrorCodes::InvalidBotSideLimitValue:
                return "InvalidBotSideLimitValue";
            case DropTargetErrorCodes::InvalidItemMaterialCode:
                return "InvalidItemMaterialCode";
            case DropTargetErrorCodes::InvalidItemMass:
                return "InvalidItemMass";
            case DropTargetErrorCodes::InvalidItemDimensions:
                return "InvalidItemDimensions";
            case DropTargetErrorCodes::RecoverableRealSenseError:
                return "RecoverableRealSenseError";
            case DropTargetErrorCodes::UnrecoverableRealSenseError:
                return "UnrecoverableRealSenseError";
            case DropTargetErrorCodes::CommandDelegateExpired:
                return "CommandDelegateExpired";
            default:
                return "UndefinedError - Not in DropTargetErrorCodes";
        }
    }

    /**
     * DropTargetError is a custom exception for any Fulfil-defined DropTargetErrorCodes to be used in the
     * drop target searching algorithms.
     */
    class DropTargetError : public std::exception {
        /* status code of the error according to the Depth Cam API Communication Cookbook */
        DropTargetErrorCodes status_code;
        /* name corresponding to the status code */
        std::string status_name;
        /* Longer message of form "Drop Target Algorithm error {STATUS CODE}: {ERROR NAME} - {this->description} "*/
        std::string message;
        /* default is empty string, and will be appended to the message field */
        std::string description;
    public:
        /**
         * DropTargetError exception to be thrown in the drop target searching algorithms
         * @param status_code denoting which error in the DropTargetErrorCodes enum is active
         * @param description optional parameter to be APPENDED to default message, for adding more detail to the error
         * and will be returned in the DropTargetResponse
         */
        explicit DropTargetError(DropTargetErrorCodes status_code,
                                 const std::string &description = "");

        [[nodiscard]] const char *what() const noexcept override;

        /**
        * Getter for the error's status code
        */
        DropTargetErrorCodes get_status_code();

        /**
        * Getter for the error's custom description
        */
        DropTargetErrorCodes get_description();
    };
} // namespace fulfil::dispense::drop_target_error_codes
#endif //FULFIL_DISPENSE_DROP_ERROR_CODES_H
