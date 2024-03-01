//
// Created by nkaffine on 12/12/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#include "Fulfil.Dispense/dispense/image_persistence/realsense_file_manager.h"
#include <Fulfil.CPPUtils/file_system_util.h>

using fulfil::dispense::imagepersistence::RealsenseFileManager;
using fulfil::dispense::commands::DropTargetRequest;
using fulfil::dispense::imagepersistence::DispenseFileManager;
using fulfil::dispense::imagepersistence::TimeStamper;
using fulfil::utils::FileSystemUtil;

RealsenseFileManager::RealsenseFileManager(int bay, std::shared_ptr<std::string> base_directory_path,
    std::shared_ptr<TimeStamper> timestamper)
{
  this->bay = bay;
  this->timestamper = timestamper;
  this->base_directory_path = std::make_shared<std::string>();
  this->base_directory_path->append(*base_directory_path);
  // Creating the parent directory of all bays if it doesn't exist.
  FileSystemUtil::create_dir_if_not_exist(this->base_directory_path->c_str());
  this->base_directory_path->append("/Bay");
  this->base_directory_path->append(std::to_string(this->bay));
  // Creating the bay specific directory if it doesn't exist.
  FileSystemUtil::create_dir_if_not_exist(this->base_directory_path->c_str());
}

std::shared_ptr<std::string> RealsenseFileManager::drop_target_filepath(std::shared_ptr<DropTargetRequest> request,
    DispenseFileManager::ImageType image_type)
{
  std::shared_ptr<std::string> filepath = std::make_shared<std::string>();
  filepath->append(*this->base_directory_path);
  filepath->append("/droptarget_");
  filepath->append(*request->command_id);
  switch (image_type)
  {
    case depth:
      filepath->append("_depth_");
      break;
    case color:
      filepath->append("_color_");
      break;
  }
  filepath->append(*this->timestamper->get_timestamp());
  filepath->append(".png");
  return filepath;
}
