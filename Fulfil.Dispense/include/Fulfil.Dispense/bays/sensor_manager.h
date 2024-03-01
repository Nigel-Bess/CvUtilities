//
// Created by nkaffine on 12/10/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_BAYS_SENSOR_MANAGER_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_BAYS_SENSOR_MANAGER_H_
#include <memory>
#include <vector>

namespace fulfil
{
namespace dispense {
namespace bays
{
/**
 * The purpose of this class is to abstract the sensor but
 * provide useful functionality to classes that don't need to
 * know about the specifics of the sensor.
 * @tparam Sensor
 */
template <class Sensor>
class SensorManager
{
 public:
  /**
   * Returns a vector of connected sensor.
   * @return pointer to vector of connected sensors.
   */
  virtual std::shared_ptr<std::vector<Sensor>> get_connected_sensors() =0;
};

} // namespace bays
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_BAYS_SENSOR_MANAGER_H_
