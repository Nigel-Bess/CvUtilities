
#ifndef FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_COMMANDS_H_
#define FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_COMMANDS_H_

#include <cstdint>

namespace fulfil::utils::networking
{
    /**
     * The purpose of this struct is to define the format that commands
     * should be sent to the SocketNetworkManager from all clients
     * regardless of what command they are sending.
     */
    struct SocketCommandHeader
    {
      /**
       * The id of the command being sent, or
       * at least the id of the header.
       */
      char command_id[24];
      /**
       * The size (in bytes) of the payload that is
       * being sent along with the header.
       */
      uint16_t bytesleft;
    };
} // namespace fulfil::utils::networking


#endif //FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_COMMANDS_H_
