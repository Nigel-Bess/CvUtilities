#ifndef FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_NETWORK_MANAGER_DELEGATE_H_
#define FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_NETWORK_MANAGER_DELEGATE_H_
#include <string>
#include <memory>

namespace fulfil::utils::networking
{
/**
 * The purpose of this class is to abstract the
 * processing of requests away from the code that handles
 * sockets. A class that implements this class will be
 * able to provide the functionality for handling requests.
 * @tparam Request type representing a request.
 */
    template <class Request>
    class SocketNetworkManagerDelegate
    {
        public:
            /**
            * Called when the socket network manager receives a valid request that should
            * be handled.
            * @param request the request from the socket network manager.
            */
            virtual void did_receive_request(Request request)=0;
            /**
            * Called when the socket network manager receives an invalid request but is
            * able to parse the request id.
            * @param request_id the id of the request that was invalid
            */
            virtual void did_receive_invalid_request(std::shared_ptr<std::string> request_id) = 0;
            /**
            * Called when the socket network manager receives and invalid request but is
            * not able to parse the request id.
            */
            virtual void did_receive_invalid_request() = 0;

            virtual void handle_request(std::shared_ptr<std::string> payload, std::shared_ptr<std::string> command_id) = 0;
    };
} // fulfil::utils::networking

#endif //FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_NETWORK_MANAGER_DELEGATE_H_
