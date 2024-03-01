//
// Created by nkaffine on 12/12/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_PARSING_COMMAND_PARSING_ERRORS_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_PARSING_COMMAND_PARSING_ERRORS_H_
#include <memory>

namespace fulfil
{
namespace dispense {
namespace commands
{
namespace errors
{
/**
 * The purpose of this class is to provide a custom exception
 * for when the command type from a request is invalid.
 */
class InvalidCommandFormatException : public std::exception
{
 public:
  /**
   * The id of the command.
   */
  std::shared_ptr<std::string> command_id;
  /**
   * InvalidCommandFormatException constructor
   * @param command_id pointer to string with the
   * command id.
   */
  explicit InvalidCommandFormatException(std::shared_ptr<std::string> command_id);

  const char* what() const throw() override;
};
} // namespace errors
} // namespace parsing
} // namespace commands
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_PARSING_COMMAND_PARSING_ERRORS_H_
