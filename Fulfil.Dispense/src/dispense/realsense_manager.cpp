//
// Created by nkaffine on 12/10/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#include "Fulfil.Dispense/dispense/realsense_manager.h"

using fulfil::dispense::RealsenseManager;
using fulfil::depthcam::DepthSession;

RealsenseManager::RealsenseManager(std::shared_ptr<fulfil::depthcam::DeviceManager> manager)
{
  this->manager = manager;
}

std::shared_ptr<std::vector<std::shared_ptr<DepthSession>>> RealsenseManager::get_connected_sensors()
{
  return this->manager->get_connected_sessions();
}
