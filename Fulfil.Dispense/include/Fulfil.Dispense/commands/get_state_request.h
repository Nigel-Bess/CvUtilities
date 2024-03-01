//
// Created by steve on 12/31/20.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_GET_STATE_GET_STATE_REQUEST_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_GET_STATE_GET_STATE_REQUEST_H_

#include <Fulfil.Dispense/commands/dispense_request.h>
#include <Fulfil.Dispense/commands/dispense_response.h>

namespace fulfil
{
namespace dispense {
namespace commands
{

class GetStateRequest : public fulfil::dispense::commands::DispenseRequest
{
 public:

  std::shared_ptr<nlohmann::json> request_json;

  explicit GetStateRequest(std::shared_ptr<std::string> command_id, std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json);

  std::shared_ptr<fulfil::dispense::commands::DispenseResponse> execute() override;
};
} // namespace commands
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_GET_STATE_GET_STATE_REQUEST_H_
