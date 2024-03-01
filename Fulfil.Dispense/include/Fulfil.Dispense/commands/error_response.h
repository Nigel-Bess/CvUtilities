//
// Created by nkaffine on 12/12/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_ERROR_RESPONSE_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_ERROR_RESPONSE_H_
#include <memory>
#include "dispense_response.h"

namespace fulfil::dispense::commands
{
/**
 * The purpose of this class is to outline
 * the functionality for repsonding with errors.
 */
class ErrorResponse : public fulfil::dispense::commands::DispenseResponse
{
 private:
  /**
   * The command id of the request if there is one.
   */
  std::shared_ptr<std::string> command_id;
  /**
   * The payload that will be sent with the response.
   */
  std::shared_ptr<std::string> payload;
  /**
   * Creates and encoding of the payload that will
   * be sent with the response and stores it in
   * the payload variable.
   */
  void encode_payload();
 public:
  /**
   * ErrorResponse constructor that initializes the error
   * response with a null value for the command id;
   */
  ErrorResponse();
  /**
   * ErrorResponse constructor that sets the command id to
   * the provided command id.
   * @param command_id pointer to string with command id.
   */
  explicit ErrorResponse(std::shared_ptr<std::string> command_id);
  /**
   * Returns the command id which will either be the value
   * provided in the constructor or null if the empty constructor
   * was used.
   * @return pointer to string with command id.
   */
  std::shared_ptr<std::string> get_command_id() override;
  /**
   * Returns the size in bytes of the payload that will
   * be sent for the response.
   * @return number of bytes in the payload.
   */
  int dispense_payload_size() override;
  /**
   * Returns a string containing the data for the payload
   * of the response.
   * @return pointer to string with payload data.
   */
  std::shared_ptr<std::string> dispense_payload() override;
};
} // namespace fulfil::dispense::commands

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_ERROR_RESPONSE_H_
