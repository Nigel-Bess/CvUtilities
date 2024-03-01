//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DISPENSE_REQUEST_PARSER_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DISPENSE_REQUEST_PARSER_H_
#include <Fulfil.CPPUtils/networking.h>
#include <Fulfil.CPPUtils/logging.h>
#include <memory>
#include <Fulfil.Dispense/commands/dispense_request.h>
#include <Fulfil.Dispense/commands/dispense_response.h>
#include <Fulfil.Dispense/commands/parsing/dispense_request_json_parser.h>


namespace fulfil
{
namespace dispense {
namespace commands
{
/**
 * The purpose of this class is to handle parsing all of
 * the dispense requests coming from the socket.
 */
class DispenseRequestParser final :
 public fulfil::utils::networking::SocketCommandParser<std::shared_ptr<fulfil::dispense::commands::DispenseRequest>>
{
 private:
  /**
   * The json parser that will be used to parse the payload of the request.
   */
  std::shared_ptr<fulfil::dispense::commands::DispenseRequestJsonParser> json_parser;

  /**
   * counter for number of heartbeat nop requests that have come through the parser
   * useful for logging a series of X commands to log traffic on DC-VLSG comms when other requests are absent
   */
  int nop_counter;

 public:
  /**
   * DispenseRequestParser constructor.
   * @param parser that will be used to parse the payload of dispense
   * requests received from the socket.
   */
  explicit DispenseRequestParser(std::shared_ptr<fulfil::dispense::commands::DispenseRequestJsonParser> parser);
  /**
   * Parses the payload received from the socket.
   * @param payload a string containing the data from the payload.
   * @param request_id a string with the id for the request.
   * @return the dispense command that was parsed from the payload.
   */
  std::shared_ptr<fulfil::dispense::commands::DispenseRequest> parse_payload(std::shared_ptr<std::string> payload,
      std::shared_ptr<std::string> request_id) override;
};
} // namespace commands
} // namespace dispense
} // namespace fulfils

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DISPENSE_REQUEST_PARSER_H_
