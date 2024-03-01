//
// Created by nkaffine on 12/9/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_INFORMATION_H_
#define FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_INFORMATION_H_
#include <memory>

namespace fulfil
{
namespace utils
{
namespace networking
{
/**
 * The purpose of this class is to encapsulate the required
 * information for creating a socket on teh machine.
 */
class SocketInformation
{
 public:
  /**
   * The port where the socket will be started.
   */
  unsigned short port;
  /**
   * SocketInformation Constructor.
   * @param port the port where the socket will be started.
   */
  SocketInformation(unsigned short port);
};
} // namespace networking
} // namespace utils
} // namespace fulfil

#endif //FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_INFORMATION_H_
