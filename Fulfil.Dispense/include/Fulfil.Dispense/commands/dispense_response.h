//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DISPENSE_RESPONSE_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DISPENSE_RESPONSE_H_
#include <Fulfil.CPPUtils/networking/socket_response.h>
#include <memory>

namespace fulfil
{
namespace dispense {
namespace commands
{
/**
 * The purpose of this class is to provide a basic outline
 * of required functionality for a response to a dispense
 * command so that it can be sent over a socket.
 */
class DispenseResponse : public fulfil::utils::networking::SocketResponse
{
 public:
  /**
   * Returns the command id for the response that will be used as
   * the id in the header of the socket response.
   * @return pointer to string with 12 byte id for the socket
   * response.
   */
  virtual std::shared_ptr<std::string> get_command_id() = 0;
  /**
   * Returns number of bytes in the dispense payload that will be
   * sent over the socket.
   * @return number of bytes in dispense payload,
   */
  virtual int dispense_payload_size() = 0;
  /**
   * Returns a string containing the data of the dispense payload
   * that will be sent over the socket.
   * @return pointer to string with data representing the dispense
   * payload.
   */
  virtual std::shared_ptr<std::string> dispense_payload() = 0;
  int header_size() final override;
  virtual void* header() final override;
  virtual int payload_size() final override;
  virtual void* payload() final override;
};
} // namespace commands
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DISPENSE_RESPONSE_H_
