//
// Created by sburke on 12/31/20.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_CODE_RESPONSE_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_CODE_RESPONSE_H_
#include <Fulfil.Dispense/commands/dispense_response.h>

namespace fulfil::dispense::commands
{

class CodeResponse : public fulfil::dispense::commands::DispenseResponse
{
 private:

  /**
   * The id for the request.
   */
  std::shared_ptr<std::string> command_id;

  /**
   *  success_code = 0 if successful, > 0 if there was an error. See  DC/VLSG API cookbook for details
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
   *  constructor that initializes a response indicating a success / failure based on error code.
   */
  explicit CodeResponse(std::shared_ptr<std::string> command_id, int success_code);

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
} // namespace fulfil

#endif //
