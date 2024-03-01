//
// Created by nkaffine on 12/12/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_IMAGE_PERSISTENCE_TIMESTAMPER_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_IMAGE_PERSISTENCE_TIMESTAMPER_H_
#include <memory>

namespace fulfil
{
namespace dispense {
namespace imagepersistence
{
/**
 * The purpose of this class is to outline functionality
 * for generating the timestamp for an image. This class is
 * abstract to all for better testing.
 */
class TimeStamper
{
 public:
  /**
   * Returns a string containing the timestamp based on the
   * implementation of the class.
   */
  virtual std::shared_ptr<std::string> get_timestamp() = 0;
};
} // namespace imagepersistence
} // namespace dispense
} // fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_IMAGE_PERSISTENCE_TIMESTAMPER_H_
