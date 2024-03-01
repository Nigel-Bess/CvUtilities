//
// Created by nkaffine on 12/12/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_IMAGE_PERSISTENCE_REALSENSE_TIMESTAMPER_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_IMAGE_PERSISTENCE_REALSENSE_TIMESTAMPER_H_
#include <Fulfil.Dispense/dispense/image_persistence/timestamper.h>
#include <string>

namespace fulfil
{
namespace dispense {
namespace imagepersistence
{
/**
 * The purpose of this class is to implement the
 * timestamper interface and provide an implementation that
 * returns the current timestamp
 */
class RealsenseTimestamper : public fulfil::dispense::imagepersistence::TimeStamper
{
 public:
  std::shared_ptr<std::string> get_timestamp() override;
};
} // namespace imagepersistence
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_IMAGE_PERSISTENCE_REALSENSE_TIMESTAMPER_H_
