//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_NOP_NOP_REQUEST_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_NOP_NOP_REQUEST_H_

#include "Fulfil.Dispense/commands/dispense_request.h"
#include "Fulfil.Dispense/commands/dispense_response.h"

namespace fulfil
{
namespace dispense {
namespace commands
{
/**
 * The purpose of this class is to provide an implementation
 * for where there is a request that should have no operation.
 */
class NopRequest : public fulfil::dispense::commands::DispenseRequest
{
 public:
  /**
   * NopRequest constructor.
   */
  explicit NopRequest(std::shared_ptr<std::string> command_id, std::shared_ptr<std::string> PrimaryKeyID);
  /**
   * Does nothing and returns a nop response
   */
  std::shared_ptr<fulfil::dispense::commands::DispenseResponse> execute() override;

};
} // namespace commands
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_NOP_NOP_REQUEST_H_
