#ifndef FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_COMMAND_PARSER_H_
#define FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_COMMAND_PARSER_H_
#include <memory>

namespace fulfil::utils::networking
{
    /**
     * The purpose of this class is to outline an abstract class that
     * will be passed to the socket network manager to allow for more
     * flexibility in parsing data from sockets.
     * @tparam Request the type representing requests.
     */
    template <class Request>
    class SocketCommandParser
    {
        public:
        /**
        * Given the payload from the socket header and the id of the socket header, it returns
        * the parsed payload if it can be parsed. If the payload is invalid, it should throw
        * an InvalidCommandFormatException.
        * @param payload the string with the payload from the header.
        * @param request_id string with the id for the header.
        * @return the request that was parsed from the payload.
        * @throws InvalidCommandFormatException if the payload is invalid.
        */
        virtual Request parse_payload(std::shared_ptr<std::string> payload, std::shared_ptr<std::string> request_id) = 0;
    };
} // namespace fulfil::utils::networking

#endif //FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_COMMAND_PARSER_H_
