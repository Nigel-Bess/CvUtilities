#ifndef FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_MANAGER_H_
#define FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_MANAGER_H_
#include <memory>
#include <netinet/in.h>
#include "socket_information.h"
#include "Fulfil.CPPUtils/comm/GrpcService.h"

namespace fulfil::utils::networking
{
/**
 * The purpose of this class is to maintain the
 * socket over it's lifecycle. It handles the creation
 * of the socket, as well as finding a connection for the socket.
 * One limitation of this class right now is that it can only
 * maintain one connection to the socket.
 */
    class SocketManager
    {
        private:
            /**
            * Pointer to the socket information that will be used to create
            * the socket.
            */
            std::shared_ptr<fulfil::utils::networking::SocketInformation> socket_information;
            /**
            * A pointer to the system object handling socket information.
            */
            std::unique_ptr<sockaddr_in> address;
        public:
            /**
            * The socket file descriptor to be used when creating
            * a connection with the socket.
            */
            int socket_fd = -1;
            /**
            * The connection file descrptor of the current connection
            * to be used for reading and writing to the connection.
            */
            int connection_fd = -1;
            /**
            * SocketManager Constructor.
            * @param socket_information pointer to the socket information that will be used
            * to create and connect to sockets with this object.
            */
            SocketManager(std::shared_ptr<fulfil::utils::networking::SocketInformation> socket_information);
            /**
            * Handles the creation of the socket.
            * @throws exception when there is an error creating the socket.
            */
            void create_socket();
            /**
            * Handles establishing a connection with the socket.
            * @throws exception when there is an error establishing
            * a connection.
            */
            void connect_socket();

            GrpcService service_;
            /**
            * Disconnect from the socket on the server side
            */
            void Disconnect();

            ~SocketManager();
    };
} // namespace fulfil::utils::networking
#endif //FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_MANAGER_H_
