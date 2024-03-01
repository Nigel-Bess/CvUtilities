//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_PARSING_DISPENSE_COMMAND_PARSER_H_
#define FULFIL_DISPENSE_INCLUDE_FULFnIL_DISPENSE_COMMANDS_PARSING_DISPENSE_COMMAND_PARSER_H_
#include <Fulfil.Dispense/commands/dispense_command.h>
#include <memory>
#include <Fulfil.Dispense/json.hpp>

namespace fulfil
{
namespace dispense {
namespace commands
{
/**
 * The purpose of this class is to parse the command
 * type from various different sources.
 */
class DispenseCommandParser
{
 public:
  /**
   * Parses the command type from an integer.
   * @param i integer representing the command type.
   * @return the command type that corresponds to the given int
   * @throws exception if the integer does not have a corresponding
   * command type.
   */
  static fulfil::dispense::commands::DispenseCommand parse(int i);
  /**
   * Parses the command type from the provided json.
   * @param json containing the command type information (not necesarrily just
   * the command type).
   * @return the command type parsed from the json
   * @throws exception if the json either doesn't contain the command information
   * or the information doesn't correspond to a command type.
   */
  static fulfil::dispense::commands::DispenseCommand parse(std::shared_ptr<nlohmann::json> json);
};
} // namespace commands
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_PARSING_DISPENSE_COMMAND_PARSER_H_
