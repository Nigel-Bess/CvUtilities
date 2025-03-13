//
// Created by jessv on 5/23/24.
//

#ifndef FULFIL_DISPENSE_TRAY_VIEW_RESPONSE_H
#define FULFIL_DISPENSE_TRAY_VIEW_RESPONSE_H

#include <Fulfil.Dispense/commands/dispense_response.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/point_3d.h>

using fulfil::utils::Point3D;

namespace fulfil::dispense::commands
{
    /**
     * The purpose of this class is to outline and contain
     * all of the information required for the response to
     * a Tray View response
     */
    class TrayViewResponse final : public fulfil::dispense::commands::DispenseResponse
    {
    private:

        /**
         * The id for the request.
         */
        std::shared_ptr<std::string> request_id;
        /**
         *  success_code = 0 if successful, > 0 if there was an error
         */
        int success_code;
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
         * constructor that initializes a response indicating a success with additional parameters to be sent
         */
        TrayViewResponse(std::shared_ptr<std::string> request_id, int success_code, std::shared_ptr<std::string> error_description);
        /**
         * Returns the command id for the response.
         * @return pointer to string containing command id for the response.
         */
        std::shared_ptr<std::string> get_command_id() override;

        int get_success_code();

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

#endif //FULFIL_DISPENSE_TRAY_VIEW_RESPONSE_H
