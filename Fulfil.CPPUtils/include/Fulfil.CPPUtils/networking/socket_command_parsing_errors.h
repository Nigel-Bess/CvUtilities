#ifndef FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_COMMAND_PARSING_ERRORS_H_
#define FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_COMMAND_PARSING_ERRORS_H_

#include <memory>

namespace fulfil::utils::networking::errors
{
/**
 * The purpose of this class is to make it easier to differentiate
 * based on exception when using sockets.
 */
    class InvalidCommandFormatException : public std::exception
    {
        public:
            /**
            * The id of the header with which the command was sent.
            */
            std::shared_ptr<std::string> command_id;
            /**
            * InvalidCommandFormatException Constructor
            * @param command_id pointer to the string with the command
            * id for the header the command came from.
            */
            InvalidCommandFormatException(std::shared_ptr<std::string> command_id)
            {
                this->command_id = command_id;
            }

            const char* what() const throw() override
            {
                return "Command Payload Invalid";
            }
    };
} // namespace fulfil::utils::networking::errors

#endif //FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_COMMAND_PARSING_ERRORS_H_
