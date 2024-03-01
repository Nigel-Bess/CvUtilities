//
// Created by nkaffine on 12/18/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
#include <memory>
#include <Fulfil.DepthCam/visualization.h>
#include <Fulfil.DepthCam/core.h>
#include <Fulfil.DepthCam/aruco.h>

int main(int argc, char** argv)
{
  std::shared_ptr<fulfil::depthcam::DeviceManager> manager = std::make_shared<fulfil::depthcam::DeviceManager>();
  std::shared_ptr<fulfil::depthcam::Session> session = manager->get_first_connected_session(-1);
  std::shared_ptr<fulfil::depthcam::visualization::SessionVisualizer> visualizer = std::make_shared<fulfil::depthcam::visualization::SessionVisualizer>(session, std::make_shared<std::string>("window"));
  std::shared_ptr<fulfil::depthcam::aruco::MarkerDetector> detector = std::make_shared<fulfil::depthcam::aruco::MarkerDetector>();
  while(true)
  {
    session->refresh();
    visualizer->draw_detected_markers(detector->dictionary, 1);
  }
  return 0;
}
