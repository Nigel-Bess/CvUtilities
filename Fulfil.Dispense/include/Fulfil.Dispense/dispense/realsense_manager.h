//
// Created by nkaffine on 12/10/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_REALSENSE_MANAGER_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_REALSENSE_MANAGER_H_
#include <Fulfil.Dispense/bays/sensor_manager.h>
#include <Fulfil.DepthCam/core.h>

namespace fulfil
{
namespace dispense {
/**
 * The purpose of this class is to encapsulate the device
 * manager from the depth cam library and use it to provide
 * the required functionality for the bays.
 */
class RealsenseManager : public fulfil::dispense::bays::SensorManager<std::shared_ptr<fulfil::depthcam::Session>>
{
 private:
  /**
   * The inner device manager that will be used to provide
   * a list of connected sensors.
   */
  std::shared_ptr<fulfil::depthcam::DeviceManager> manager;
 public:
  /**
   * RealsenseManager constructor
   * @param manager the device manager that will be used to
   * generate the list of connected sensors.
   */
  explicit RealsenseManager(std::shared_ptr<fulfil::depthcam::DeviceManager> manager);
  /**
   * Returns a list of all of the connected sensors.
   * @return pointer to a vector of sessions from connected sesors.
   */
  std::shared_ptr<std::vector<std::shared_ptr<fulfil::depthcam::Session>>> get_connected_sensors() override;
};
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_REALSENSE_MANAGER_H_
