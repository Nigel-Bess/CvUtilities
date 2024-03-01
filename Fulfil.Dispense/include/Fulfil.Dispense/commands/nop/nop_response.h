//
// Created by steve on 2/25/21.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_NOP_NOP_RESPONSE_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_NOP_NOP_RESPONSE_H_
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
 * a drop target response
 */
class NopResponse : public fulfil::dispense::commands::DispenseResponse
{
 private:

  /**
   * The id for the request.
   */
  std::shared_ptr<std::string> command_id;

  /**
   * status of rail motor
   */
  bool rail_motor_stationary;
  /**
   *  success_code = 0 if successful, > 0 if there was an error
   */
  int success_code;

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
  NopResponse(std::shared_ptr<std::string> command_id, int success_code, bool rail_motor_stationary);
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
} // namespace commands
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_SRC_COMMANDS_NOP_NOP_RESPONSE_H_
