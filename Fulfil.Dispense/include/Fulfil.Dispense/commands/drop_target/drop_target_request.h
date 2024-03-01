//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DROP_TARGET_DROP_TARGET_REQUEST_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DROP_TARGET_DROP_TARGET_REQUEST_H_
#include <Fulfil.Dispense/commands/dispense_request.h>
#include <Fulfil.Dispense/commands/dispense_response.h>
#include <Fulfil.Dispense/commands/drop_target/drop_target_details.h>

namespace fulfil
{
namespace dispense {
namespace commands
{
/**
 * The purpose of this class is to implement the dispense command
 * interface for drop target requests which scan the bag for drop locations
 * and returns a drop center (in addition to saving images if desired).
 */
class DropTargetRequest : public fulfil::dispense::commands::DispenseRequest
{
 private:
  /**
   * The details required to perform the drop target command.
   */
  std::shared_ptr<fulfil::dispense::commands::DropTargetDetails> details;
  std::shared_ptr<nlohmann::json> request_json;
 public:
  /**
   * DropTargetRequest constructor.
   * @param command_id pointer to string with the command id for the request.
   * @param details pointer to the details for the drop target request.
   */
  DropTargetRequest(std::shared_ptr<std::string> command_id,
                 std::shared_ptr<std::string> PrimaryKeyID,
                 std::shared_ptr<DropTargetDetails> details,
                 std::shared_ptr<nlohmann::json> request_json);
  /**
   * Executes the drop target request by calling the delegate and providing
   * the details for the drop target request.
   * @return the dispense response from executing the drop target request.
   */
  std::shared_ptr<fulfil::dispense::commands::DispenseResponse> execute() override;

};
} // namespace commands
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DROP_TARGET_DROP_TARGET_REQUEST_H_
