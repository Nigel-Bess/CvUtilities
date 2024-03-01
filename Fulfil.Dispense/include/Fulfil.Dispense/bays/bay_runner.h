//
// Created by nkaffine on 12/10/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_BAYS_BAY_RUNNER_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_BAYS_BAY_RUNNER_H_
#include <memory>

namespace fulfil
{
namespace dispense {
namespace bays
{
/**
 * The purpose of this class is to provide an interface for
 * bays to kick off a generic function for a bay.
 */
class BayRunner
{
 public:
  /**
   * Starts the execution of the function for the bay.
   */
  virtual void start() = 0;
  /**
   * Due to the nature of shared and weak pointers, there must
   * be a shared pointer to an object before you can create a
   * weak pointer to it. This function is meant to be called
   * after there is a shared pointer to it so that it can pass
   * a weak pointer of itself as the delegate to whatever
   * objects it needs to.
   */
  virtual void bind_delegates() = 0;
  /**
   * Returns a shared pointer of this bay runner.
   * @return shared pointer to this bay runner.
   * @note this function is necessary because making
   * BayRunner inherit from enable_shared_from_this causes
   * conflicts if you want to get a shared pointer from a
   * child class as the child class. You can only inherit from
   * enable_shared_from_this for one type or else it is ambiguous
   * so any object that inherits this would only be able to return
   * shared pointers to a bay runner instead of the child class.
   */
  virtual std::shared_ptr<BayRunner> get_shared_ptr() = 0;
};
} // namespace bays
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_BAYS_BAY_RUNNER_H_
