#ifndef FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_RESPONSE_H_
#define FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_RESPONSE_H_
#include "Fulfil.CPPUtils/comm/depthCams.pb.h"

namespace fulfil::utils::networking
{
/**
 * The purpose of this class is to define the requirements for
 * transmitting data through a socket.
 */
    class SocketResponse
    {
        public:
            /**
            * The size of the header that will
            * be sent in bytes.
            * @return size of header in bytes.
            */
            virtual int header_size() = 0;
            /**
            * Returns a pointer to the data of the header. This should
            * be a pointer that has been malloced so it can be freed by
            * the SocketNetworkManager.
            * @return pointer to the header on the heap.
            */
            virtual void* header() = 0;
            /**
            * Returns the size in bytes of the payload that will be sent via
            * the socket.
            * @return size in bytes of the payload.
            */
            virtual int payload_size() = 0;
            /**
            * Returns a pointer to the data of the payload. This
            * should be a pointer that has been malloced so it can
            * be freed by the SocketNetworkManager.
            * @return pointer to the payload on the heap.
            */
            virtual void* payload() = 0;

            DepthCameras::MessageType message_type = DepthCameras::MessageType::MESSAGE_TYPE_GENERIC_QUERY;

    };
} // namespace fulfil::utils::networking

#endif //FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_RESPONSE_H_
