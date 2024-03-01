//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_START_TRAY_VIDEO_START_TRAY_VIDEO_REQUEST_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_START_TRAY_VIDEO_START_TRAY_VIDEO_REQUEST_H_

#include <Fulfil.Dispense/commands/dispense_request.h>
#include <Fulfil.Dispense/commands/dispense_response.h>

namespace fulfil
{
namespace dispense {
namespace commands
{
/**
 * The purpose of this class is to provide an implementation
 * of a request for starting saving video data for a tray session.
 */
class StartTrayVideoRequest : public fulfil::dispense::commands::DispenseRequest
{
 public:
  /**
   * StartTrayVideoRequest constructor.
   * @param command_id of the request.
   */
  explicit StartTrayVideoRequest(std::shared_ptr<std::string> command_id, std::shared_ptr<std::string> PrimaryKeyID);
  /**
   * Calls method in dispense_manager to start saving data from current tray session
   */
  std::shared_ptr<fulfil::dispense::commands::DispenseResponse> execute() override;
};
} // namespace commands
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_START_TRAY_VIDEO_START_TRAY_VIDEO_REQUEST_H_
