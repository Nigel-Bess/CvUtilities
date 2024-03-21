#ifndef FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_INFORMATION_H_
#define FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_INFORMATION_H_
#include <memory>

namespace fulfil::utils::networking
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
} // namespace fulfil::utils::networking

#endif //FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_INFORMATION_H_
