//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#include <Fulfil.Dispense/commands/parsing/dispense_json_parser.h>
#include <Fulfil.Dispense/commands/drop_target/json_item.h>
#include <Fulfil.Dispense/commands/drop_target/drop_target_details.h>
#include <iostream>

using fulfil::dispense::commands::DispenseJsonParser;
using fulfil::dispense::commands::DropTargetDetails;

std::shared_ptr<DropTargetDetails> DispenseJsonParser::parse_drop_target(
    std::shared_ptr<nlohmann::json> request_json,
    std::shared_ptr<std::string> command_id)
{
  return std::make_shared<DropTargetDetails>(request_json, command_id);
}