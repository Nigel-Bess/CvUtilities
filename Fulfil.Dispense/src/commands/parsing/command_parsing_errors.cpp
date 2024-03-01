//
// Created by nkaffine on 12/12/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#include "Fulfil.Dispense/commands/parsing/command_parsing_errors.h"

using fulfil::dispense::commands::errors::InvalidCommandFormatException;

InvalidCommandFormatException::InvalidCommandFormatException(std::shared_ptr<std::string> command_id)
{
  this->command_id = command_id;
}

const char *InvalidCommandFormatException::what() const throw()
{
  return "Command Payload Invalid";
}
