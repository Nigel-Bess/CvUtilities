//
// Created by amber on 6/20/24.
//

#ifndef FULFIL_COMPUTERVISION_POST_SIDE_DISPENSE_RESPONSE_H
#define FULFIL_COMPUTERVISION_POST_SIDE_DISPENSE_RESPONSE_H
namespace fulfil::dispense::commands {
    /**
     *
     */
    class PostSideDispenseResponse : public fulfil::dispense::commands::DispenseResponse
    {
    private:

        /**
         * The id for the request.
         */
        std::shared_ptr<std::string> command_id;
        int items_dispensed{1};
        int bag_full_percent{0};
        int item_on_target_percent{0};
        std::vector<int> products_to_overflow{};

        /**
         *  success_code = 0 if successful, > 0 if there was an error
         */
        int success_code{0};
        /**
         * Description of the error code thrown. Will be empty string if code is success.
         */
        std::string error_description{};
        /**
         * The payload to be sent in response to the request
         */
        std::string payload;
        /**
         * Encodes the payload in the payload string variable on this object.
         */
        void encode_payload();
    public:
        /**
         * PostSideDispenseResponse constructor that initializes a response including target information
         * @param command_id of the request that led to this response.
         * See above for descriptions of the other params
         */
        PostSideDispenseResponse(std::shared_ptr<std::string> command_handshake_id);

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

#endif //FULFIL_COMPUTERVISION_POST_SIDE_DISPENSE_RESPONSE_H
