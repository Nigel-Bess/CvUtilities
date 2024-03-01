//
// Created by steve on 5/11/20.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TRAY_VALIDATION_RESPONSE_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TRAY_VALIDATION_RESPONSE_H_

#include <Fulfil.Dispense/commands/dispense_response.h>
#include <Fulfil.CPPUtils/logging.h>

namespace fulfil
{
namespace dispense {
namespace commands
{
/**
 * The purpose of this class is to outline and contain
 * all of the information required for the response to
 * a post drop response
 */
class TrayValidationResponse final : public fulfil::dispense::commands::DispenseResponse
{
 private:

  /**
   * The id for the request.
   */
  std::shared_ptr<std::string> command_id;

  /**
   * The payload to be sent in response to the request
   */
  std::shared_ptr<std::string> payload;


 public:
  /**
   * TrayValidationResponse constructor that initializes a response indicating a failure.
   * @param command_id the command id of the request that led to this response
   * @param success_code indicates what kind of error took place. > 0 means an error
   */
  explicit TrayValidationResponse(std::shared_ptr<std::string> command_id, std::shared_ptr<std::string> payload);

  explicit TrayValidationResponse(std::shared_ptr<std::string> command_id, std::shared_ptr<std::string> payload,
                                  int first_item_edge_distance);

    /**
     * TrayValidationResponse constructor that initializes a response indicating a failure.
     * @param command_id the command id of the request that led to this response
     * @param success_code indicates what kind of error took place. > 0 means an error
     */
  explicit TrayValidationResponse(std::shared_ptr<std::string> command_id, int success_code);

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

    /**
       * distance in mm to edge of first item in lane
       */
    int first_item_edge_distance;
};
} // namespace commands
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TRAY_VALIDATION_RESPONSE_H_
