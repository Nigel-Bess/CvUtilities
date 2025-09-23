//
// Created by amber on 6/20/24.
//

#ifndef FULFIL_COMPUTERVISION_SIDE_DISPENSE_TARGET_RESPONSE_H
#define FULFIL_COMPUTERVISION_SIDE_DISPENSE_TARGET_RESPONSE_H
#include <Fulfil.Dispense/commands/dispense_response.h>
#include <Fulfil.CPPUtils/logging.h>

namespace fulfil::dispense::commands {
    class SideDispenseTargetResponse final : public fulfil::dispense::commands::DispenseResponse
    {
    private:

        /**
         * The id for the request.
         */
        std::shared_ptr<std::string> command_id;
        float x{0};
        float y{0};
        float extend_contact_distance{0};

        /**
         *  success_code = 0 if successful, > 0 if there was an error
         */
        int success_code{0};
        /**
         * Description of the error code thrown. Will be empty string if code is success.
         */
        std::shared_ptr<std::string> error_description;
        /**
         * The payload to be sent in response to the request
         */
        std::shared_ptr<std::string> payload;
        /**
         * Encodes the payload in the payload string variable on this object.
         */
        void encode_payload();
    public:
        /**
         * SideDispenseTargetResponse constructor that initializes a response indicating a failure.
         * @param command_id the command id of the request that led to this response
         * @param success_code indicates what kind of error took place. > 0 means an error
         */
        explicit SideDispenseTargetResponse(std::shared_ptr<std::string> command_handshake_id, 
                                           int success_code = 0, 
                                           std::shared_ptr<std::string> error_desc = nullptr);
        /**
         * Returns the command id for the response.
         * @return pointer to string containing command id for the response.
         */
        std::shared_ptr<std::string> get_command_id() override;
        /**
         * Returns the size (in bytes) of the payload containing information about the drop result.
         * @return size (in bytes) of the payload to be sent.
         */
        int dispense_payload_size() override;
        /**
         * Returns the payload containing information about the drop result.
         * @return pointer to string containing data representing the drop result.
         */
        std::shared_ptr<std::string> dispense_payload() override;
    };
} // namespace fulfil::dispense::commands

#endif //FULFIL_COMPUTERVISION_SIDE_DISPENSE_TARGET_RESPONSE_H
