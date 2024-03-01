//
// Created by nkaffine on 12/9/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_ERRORS_H_
#define FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_ERRORS_H_
#include <memory>
#include "Fulfil.CPPUtils/logging.h"

namespace fulfil
{
namespace utils
{
namespace networking
{
namespace errors
{
/**
 * The purpose of this class is to provide an error
 * type specific to when the header that was sent
 * was malformed.
 */
class SocketHeaderParsingException : public std::exception
{
 public:
  const char *what() const throw() override
  {
    return "Failed to Parse Header";
  }
};

/**
 * The purpose of this class is to provide an error
 * type specific to when the socket gets disconnected
 * unexpectedly.
 */
class SocketDisconnectionException : public std::exception
{
 public:
  const char *what() const throw() override
  {
    return "Socket Disconnected";
  }
};

/**
 * The purpose of this class is to provide an error
 * type specific to when a payload read is not the
 * expected size from the header. It stores some
 * helpful information for debugging
 */
class UnexpectedSocketReadSizeException : public std::exception
{
 public:
  /**
   * The id of the header of the payload where
   * the exception was raised
   */
  std::shared_ptr<std::string> command_id;
  /**
   * The number of bytes that were read from the socket
   * for the payload alone.
   */
  long bytes_read;
  /**
   * The number of bytes that were expected to be read
   * from the socket.
   */
  long expected_bytes_read;
  /**
   * UnexpectedSocketReadSizeException Constructor
   * @param command_id string with the command id of the header the
   * exception was raised from.
   * @param bytes_read actual bytes read from the socket
   * @param expected_bytes_read expected bytes to read based on the header.
   */
  UnexpectedSocketReadSizeException(std::shared_ptr<std::string> command_id, long bytes_read, long expected_bytes_read)
  {
    this->command_id = command_id;
    this->bytes_read = bytes_read;
    this->expected_bytes_read = expected_bytes_read;
  }

  const char *what() const throw() override
  {

      fulfil::utils::Logger::Instance()->Error("Unexpected Size Read From Socket; Vars: command_id = {}, bytes_expected = {}, "
                                               "bytes_read = {}", *this->command_id, std::to_string(expected_bytes_read), std::to_string(bytes_read));

    return "Unexpected Size Read From Socket";
  }
};

/**
 * The purpose of this class is to provide an error
 * type for when the bytes left section of the header
 * contains an invalid value such as one less than 0.
 */
class InvalidSocketPayloadSizeException : public std::exception
{
 public:
  /**
   * The id of the header where the size error
   * ocurred.
   */
  std::shared_ptr<std::string> command_id;
  /**
   * InvalidSocketPayloadSizeException Constructor.
   * @param command_id string with command id of header
   * that caused the error.
   */
  InvalidSocketPayloadSizeException(std::shared_ptr<std::string> command_id)
  {
    this->command_id = command_id;
  }

  const char *what() const throw() override
  {
    return "Invalid Size Payload From Header";
  }
};
} // namespace errors
} // namespace networking
} // namespace utils
} // namespace fulfil

#endif //FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_ERRORS_H_
