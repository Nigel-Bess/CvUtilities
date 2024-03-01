//
// Created by steve on 5/11/20.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TRAY_VALIDATION_REQUEST_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TRAY_VALIDATION_REQUEST_H_

#include <Fulfil.Dispense/commands/dispense_request.h>
#include <Fulfil.Dispense/commands/dispense_response.h>
#include <Fulfil.Dispense/commands/tray_validation/tray_validation_details.h>

namespace fulfil
{
namespace dispense {
namespace commands
{
/**
 * The purpose of this class is to implement the tray height request interface
 */
class TrayValidationRequest : public fulfil::dispense::commands::DispenseRequest
{
 private:
  /**
   * The details required to perform the pre drop command.
   */
    std::shared_ptr<nlohmann::json> request_json;
 public:
  /**
   * TrayValidationRequest constructor.
   */
  TrayValidationRequest(std::shared_ptr<std::string> command_id, std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json);
  /**
   * Executes the tray height request by calling the delegate and providing
   * the details for the tray height request.
   */
  std::shared_ptr<fulfil::dispense::commands::DispenseResponse> execute() override;
};
} // namespace commands
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TRAY_VALIDATION_REQUEST_H_
