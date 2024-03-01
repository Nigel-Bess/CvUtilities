//
// Created by nkaffine on 12/12/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_IMAGE_PERSISTENCE_REALSENSE_FILE_MANAGER_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_IMAGE_PERSISTENCE_REALSENSE_FILE_MANAGER_H_
#include <Fulfil.Dispense/dispense/image_persistence/dispense_file_manager.h>
#include <Fulfil.Dispense/dispense/image_persistence/timestamper.h>

namespace fulfil
{
namespace dispense {
namespace imagepersistence
{
/**
 * The purpose of this class is to provide an
 * implementation of the dispense file manager interface
 * so each bay can create files in the correct directory.
 */
class RealsenseFileManager : public fulfil::dispense::imagepersistence::DispenseFileManager
{
 private:
  /**
   * The path to the directory where all of the persisted
   * images will be stored. (they will actually be stored
   * in subdirectories of this directory).
   */
  std::shared_ptr<std::string> base_directory_path;
  /**
   * The identifier for the bay of this file manager.
   */
  int bay;
  /**
   * The timestamper that will be used to create the
   * timestamp for the image file names.
   */
  std::shared_ptr<fulfil::dispense::imagepersistence::TimeStamper> timestamper;
 public:
  /**
   * RealsenseFileManager constructor
   * @param bay the id of the bay this file manager manages.
   * @param base_directory_path pointer to string containing path
   * to the directory where images will be stored.
   * @param timestamper the object that will create timestamps for filenames.
   * @note this function will create the directory passed if it does not
   * exist.
   * @throws exception if the parent directory of the provided path does
   * not exist.
   */
  RealsenseFileManager(int bay, std::shared_ptr<std::string> base_directory_path, std::shared_ptr<fulfil::dispense::imagepersistence::TimeStamper> timestamper);
  /**
   * Returns the filename for a pre drop request image based on the given request
   * and type.
   * @param request pointer to object with information about the request
   * @param image_type the type of image that is being saved.
   * @return pointer to string with the filepath the image will be saved in.
   * @note this function will handle the creation of any subdirectories that are
   * needed for the structure it returns. If this function returns successfully,
   * the directory where the file will be stored will exist.
   */
  std::shared_ptr<std::string> drop_target_filepath(std::shared_ptr<fulfil::dispense::commands::DropTargetRequest> request,
                                               ImageType image_type) override;
};
} // namespace imagepersistence
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_IMAGE_PERSISTENCE_REALSENSE_FILE_MANAGER_H_
