//
// Created by nkaffine on 12/12/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_IMAGE_PERSISTENCE_REALSENSE_IMAGE_PERSISTENCE_MANAGER_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_IMAGE_PERSISTENCE_REALSENSE_IMAGE_PERSISTENCE_MANAGER_H_
#include <Fulfil.Dispense/dispense/image_persistence/dispense_image_persistence_manager.h>
#include <Fulfil.Dispense/dispense/image_persistence/dispense_file_manager.h>
#include <Fulfil.DepthCam/core.h>

namespace fulfil
{
namespace dispense {
namespace imagepersistence
{
/**
 * The purpose of this class is to provide an implementation
 * of the dispense image persistence manager to handle saving
 * all images related to requests.
 */
class RealsenseImagePersistenceManager : public fulfil::dispense::imagepersistence::DispenseImagePersistenceManager
{
 private:
  /**
   * The file manager that will manage where the files are stored by
   * providing the filepaths.
   */
  std::shared_ptr<fulfil::dispense::imagepersistence::DispenseFileManager> file_manager;
  std::shared_ptr<fulfil::depthcam::Session> session;
 public:
  /**
   * RealsenseImagePersistenceManager constructor
   * @param file_manager that determines where all of the file are stored.
   */
  RealsenseImagePersistenceManager(std::shared_ptr<fulfil::dispense::imagepersistence::DispenseFileManager> file_manager,
      std::shared_ptr<fulfil::depthcam::Session> session);
  /**
   * Saves the images required for the pre drop request.
   * @param request pointer to details about the request being saved.
   */
  void persist_drop_target(std::shared_ptr<fulfil::dispense::commands::DropTargetRequest> request) override;
};
} // namespace imagepersistence
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_IMAGE_PERSISTENCE_REALSENSE_IMAGE_PERSISTENCE_MANAGER_H_
