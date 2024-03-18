//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DISPENSE_REQUEST_JSON_PARSER_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DISPENSE_REQUEST_JSON_PARSER_H_
#include <memory>
#include <Fulfil.Dispense/commands/drop_target/drop_target_details.h>
#include <json.hpp>

namespace fulfil
{
namespace dispense {
namespace commands
{
/**
 * The purpose of this class is to outline requirements for
 * objects that will be used to retrieve information from
 * json sent via the sockets.
 */
class DispenseRequestJsonParser
{
 public:
  /**
   * Returns the pre drop details from the given json string.
   * @param request_json string of json with details of the request.
   * @param command_id of the request this command came from.
   * @return pointer to pre drop details parsed from the json.
   * @throws exception when the json data is malformed.
   */
  virtual std::shared_ptr<fulfil::dispense::commands::DropTargetDetails> parse_drop_target(
      std::shared_ptr<nlohmann::json> request_json,
      std::shared_ptr<std::string> command_id) = 0;
};
} // namespace commands
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DISPENSE_REQUEST_JSON_PARSER_H_
