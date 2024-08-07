//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_POST_DROP_REQUEST_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_POST_DROP_REQUEST_H_

#include <Fulfil.Dispense/commands/dispense_request.h>
#include <Fulfil.Dispense/commands/dispense_response.h>

namespace fulfil
{
namespace dispense {
namespace commands
{

class PostLFRRequest : public fulfil::dispense::commands::DispenseRequest
{
 public:

  std::shared_ptr<nlohmann::json> request_json;

  explicit PostLFRRequest(std::shared_ptr<std::string> command_id, std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json);

  std::shared_ptr<fulfil::dispense::commands::DispenseResponse> execute() override;
};
} // namespace commands
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_POST_DROP_REQUEST_H_
