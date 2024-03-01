//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_PARSING_DISPENSE_JSON_PARSER_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_PARSING_DISPENSE_JSON_PARSER_H_

#include "dispense_request_json_parser.h"
namespace fulfil
{
namespace dispense {
namespace commands
{
/**inter
 * The purpose of this class is to implement the
 * dispense request json parser interface
 */
class DispenseJsonParser final : public fulfil::dispense::commands::DispenseRequestJsonParser
{

 public:
  /**
   * Parses the given json and command id into drop_target details
   * @param request_json the json for the request that contains information
   * for the drop details.
   * @param command_id of the request the command came from.
   * @return pointer to pre drop details parsed from the json (item dimensions are in mm units)
   * @throws exception if the json is malformed.
   */
  std::shared_ptr<fulfil::dispense::commands::DropTargetDetails> parse_drop_target(std::shared_ptr<nlohmann::json> request_json,
      std::shared_ptr<std::string> command_id) override ;
};
} // namespace commands
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_PARSING_DISPENSE_JSON_PARSER_H_
