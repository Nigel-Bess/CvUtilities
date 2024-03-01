//
// Created by nkaffine on 12/10/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_BAYS_BAY_RUNNER_FACTORY_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_BAYS_BAY_RUNNER_FACTORY_H_
#include <memory>
#include <Fulfil.Dispense/bays//bay_runner.h>

namespace fulfil
{
namespace dispense {
namespace bays
{
/**
 * The purpose of this class is to provide an interface
 * for creating bay runners, this was much simpler than
 * having a required constructor for the BayRunner
 * @tparam Sensor class representing a sensor.
 */
template <class Sensor>
class BayRunnerFactory
{
 public:
  /**
   * Creates a bay runner with the given bay number and sensor.
   * @param bay_num the number identifier of the bay this runner
   * belongs to.
   * @param sensor the sensor that is connected to the bay.
   * @return the bay runner based on the provided bay identifier and
   * the given sensor.
   */
  virtual std::shared_ptr<fulfil::dispense::bays::BayRunner> create(int bay_num, Sensor sensor1, Sensor sensor2) = 0;
  /**
   * Creates a bay runner when there is no sensor available for the runner.
   * @param bay_num the number identifier of the bay this runner
   * belongs to.
   * @return the bay runner based on the bay identifier.
   */
  virtual std::shared_ptr<fulfil::dispense::bays::BayRunner> create_empty(int bay_num) = 0;
};
} // namespace bays
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_BAYS_BAY_RUNNER_FACTORY_H_
