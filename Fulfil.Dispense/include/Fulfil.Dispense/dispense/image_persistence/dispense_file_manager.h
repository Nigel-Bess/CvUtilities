//
// Created by nkaffine on 12/12/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_IMAGE_PERSISTENCE_DISPENSE_FILE_MANAGER_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_IMAGE_PERSISTENCE_DISPENSE_FILE_MANAGER_H_
#include <memory>
#include <Fulfil.Dispense/commands/drop_target/drop_target_request.h>

namespace fulfil
{
namespace dispense {
namespace imagepersistence
{
/**
 * The purpose of this class is to outline functionality
 * to determine file names for images saved from requests.
 * This class is abstract to allow for easier testing.
 */
class DispenseFileManager
{
 public:
  /**
   * Enumeration to represent the different
   * types of images that can be stored from
   * a request.
   */
  enum ImageType
  {
    /// An image with depth data.
    depth,
    /// An image with color data.
    color
  };
  /**
   * Returns the filepath for a drop_target image based on the given
   * request and the given image type.
   * @param request pointer to request with details for filename.
   * @param image_type the type of the image that will be stored.
   * @return pointer to a string with filepath for the persisted file.
   */
  virtual std::shared_ptr<std::string> drop_target_filepath(std::shared_ptr<fulfil::dispense::commands::DropTargetRequest> request,
      ImageType image_type) = 0;
};
} // namespace imagepersistence
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_IMAGE_PERSISTENCE_DISPENSE_FILE_MANAGER_H_
