//
// Created by nkaffine on 12/10/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_BAYS_BAY_PARSER_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_BAYS_BAY_PARSER_H_

#include <Fulfil.DepthCam/core.h>
#include <Fulfil.DepthCam/core/depth_session.h>

using fulfil::depthcam::DepthSession;

namespace fulfil
{
namespace dispense {
namespace bays
{
/**
 * The purpose of this class is to determine which sensor
 * should go to which bay based on information in the
 * sensor.
 * @tparam Sensor class to represent a sensor.
 */
class BayParser
{
 public:
  /**
   * Returns the identifier of the bay the given
   * sensor should go to.
   * @param sensor being assigned to a bay
   * @return the identifier of the bay the
   * sensor should go to.
   */
  virtual std::vector<std::shared_ptr<DepthSession>> get_bay(std::shared_ptr<std::vector<std::shared_ptr<fulfil::depthcam::DepthSession>>> sessions,
     const std::string &config_id) = 0;

  virtual std::vector<std::string> get_bay_ids ()= 0;
};
} // namespace bays
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_BAYS_BAY_PARSER_H_
