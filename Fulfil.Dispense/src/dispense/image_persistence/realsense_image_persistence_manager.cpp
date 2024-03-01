//
// Created by nkaffine on 12/12/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#include "Fulfil.Dispense/dispense/image_persistence/realsense_image_persistence_manager.h"
#include <opencv2/opencv.hpp>

using fulfil::dispense::imagepersistence::RealsenseImagePersistenceManager;
using fulfil::dispense::imagepersistence::DispenseFileManager;
using fulfil::dispense::commands::DropTargetRequest;
using fulfil::depthcam::Session;

RealsenseImagePersistenceManager::RealsenseImagePersistenceManager(std::shared_ptr<DispenseFileManager> file_manager,
                                                                   std::shared_ptr<Session> session)
{
  this->file_manager = file_manager;
  this->session = session;
}

void RealsenseImagePersistenceManager::persist_drop_target(std::shared_ptr<DropTargetRequest> request)
{
  // Saving the color image
  std::shared_ptr<std::string> color_filepath = this->file_manager->drop_target_filepath(request, DispenseFileManager::color);
  cv::imwrite(color_filepath->c_str(), *this->session->get_color_mat());
  // Saving the depth image
  std::shared_ptr<std::string> depth_filepath = this->file_manager->drop_target_filepath(request, DispenseFileManager::depth);
  cv::imwrite(depth_filepath->c_str(), *this->session->get_depth_mat());
}
