//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_START_LFB_VIDEO_START_LFB_VIDEO_REQUEST_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_START_LFB_VIDEO_START_LFB_VIDEO_REQUEST_H_

#include <Fulfil.Dispense/commands/dispense_request.h>
#include <Fulfil.Dispense/commands/dispense_response.h>

namespace fulfil
{
namespace dispense {
namespace commands
{
/**
 * The purpose of this class is to provide an implementation
 * of a request for starting saving video data for an LFB session.
 */
class StartLFBVideoRequest : public fulfil::dispense::commands::DispenseRequest
{
 public:
  /**
   * StartLFBVideoRequest constructor.
   * @param command_id of the request.
   */
  explicit StartLFBVideoRequest(std::shared_ptr<std::string> command_id, std::shared_ptr<std::string> PrimaryKeyID);
    /**
   * Calls method in dispense_manager to start saving data from current LFB session
   */
  std::shared_ptr<fulfil::dispense::commands::DispenseResponse> execute() override;
};
} // namespace commands
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_START_LFB_VIDEO_START_LFB_VIDEO_REQUEST_H_
