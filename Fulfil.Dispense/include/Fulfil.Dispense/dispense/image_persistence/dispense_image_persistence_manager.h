//
// Created by nkaffine on 12/12/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_IMAGE_PERSISTENCE_REQUEST_IMAGE_FILE_MANAGER_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_IMAGE_PERSISTENCE_REQUEST_IMAGE_FILE_MANAGER_H_
#include <memory>
#include <Fulfil.Dispense/commands/drop_target/drop_target_request.h>

namespace fulfil
{
namespace dispense {
namespace imagepersistence
{
/**
 * The purpose of this class is to outline the
 * functionality for a class that will manage
 * saving any images related to dispense requests.
 */
class DispenseImagePersistenceManager
{
 public:
  /**
   * Saves the depth and color data for the pre drop
   * request.
   * @param request pointer to object with details about
   * the pre drop request.
   */
  virtual void persist_drop_target(std::shared_ptr<fulfil::dispense::commands::DropTargetRequest> request) = 0;
};
} // namespace imagepersistence
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_IMAGE_PERSISTENCE_REQUEST_IMAGE_FILE_MANAGER_H_
