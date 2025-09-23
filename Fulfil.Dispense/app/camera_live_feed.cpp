//
// Created by nkaffine on 12/17/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
#include <Fulfil.Dispense/drop.h>
#include <Fulfil.DepthCam/core.h>
#include <Fulfil.DepthCam/aruco.h>
#include <Fulfil.DepthCam/mocks.h>
#include <Fulfil.DepthCam/point_cloud.h>
#include <eigen3/Eigen/Geometry>
#include <Fulfil.CPPUtils/logging.h>
#include <memory>
#include <Fulfil.DepthCam/visualization.h>
#include <iostream>
#include <unistd.h>
#include <cstring>

using fulfil::utils::Logger;

[[noreturn]] void display_session(std::shared_ptr<fulfil::depthcam::Session> current_session, std::string cam_type, std::string tray_type)
{
  std::string serial_number {*current_session->get_serial_number() };
  std::cout << "The current serial_number camera to display is: " << serial_number << '\n';
  std::shared_ptr<INIReader> reader = std::make_shared<INIReader>("AGX_specific_main.ini", true);

  // If the tray hasn't been fully calibrated, skip displaying the pixels rather than throwing an error
  bool skip_pixel_display = false;
  int calibration_line_y = 0;
  std::vector <std::shared_ptr<cv::Point2f>> pixels;

  if(cam_type != "LFB") {
    std::shared_ptr <INIReader> tray_config_reader = std::make_shared<INIReader>("tray_config.ini", true);
    std::string tray_section = "tray_dimensions_" + tray_type;
    calibration_line_y = tray_config_reader->GetInteger(tray_section, "calibration_line_height", -1);
    if (calibration_line_y==-1)  Logger::Instance()->Error("Could not read calibration line value from tray config, check configs");

    try {
      std::shared_ptr <INIReader> tray_calib_reader = std::make_shared<INIReader>("tray_calibration_data_dispense.ini", true);;
      std::string section = std::string(serial_number) + "_pixel_locations";
      if (cam_type == "Induction"){
          tray_calib_reader = std::make_shared<INIReader>("tray_calibration_data_induction.ini", true);
          section = serial_number + "_induction_pixel_locations";
      }

      // Get the pixel points from the tray calibration ini
      std::vector<float> x_dims;
      tray_calib_reader->FillFloatVector(section, "x", x_dims);

      std::vector<float> y_dims;
      tray_calib_reader->FillFloatVector(section, "y", y_dims);

      for (int i = 0; i < 4; i++) {
        pixels.push_back(std::make_shared<cv::Point2f>(cv::Point2f(x_dims[i], y_dims[i])));
      }
    }
    catch (const std::exception& e) {
      std::cout << "Error parsing pixel calibration data from config ini for camera, skipping pixel display" << std::endl;
      skip_pixel_display = true;
    }
  }

  // Create the session visualizer
  std::shared_ptr<std::string> window_name = std::make_shared<std::string>("Visualizer Window");
  std::shared_ptr<std::pair <int, int>> window_location_0 = std::make_shared<std::pair <int, int>>(0,0);
  std::shared_ptr<std::pair <int, int>> window_size_0 = std::make_shared<std::pair <int, int>>(1280,720);
  int wait_time_0 = 1000;

  std::shared_ptr<fulfil::depthcam::visualization::SessionVisualizer> session_visualizer =
          std::make_shared<fulfil::depthcam::visualization::SessionVisualizer>(current_session, window_name, window_location_0, window_size_0, wait_time_0);

  // Begin live streaming
  while (true)
  {
    current_session->refresh(true,false); //update frames, do not validate them - want to display even if invalid data present.
    std::shared_ptr<cv::Mat> RGB_matrix = current_session->get_color_mat();
    // If this is an LFB camera, display the red crosshairs
    if (cam_type == "LFB"){
        session_visualizer->display_rgb_image_with_crosshair(RGB_matrix);
    }
    // If this is a Tray camera, display fiducial points from tray calibration config and horizontal green line
    else {
      // display green line for calibration to line up with white cross beam on floor of VLS
      if (calibration_line_y!=-1) {
          cv::Point p1(0, calibration_line_y);
          cv::Point p2(1280, calibration_line_y);
          cv::line(*RGB_matrix, p1, p2, cv::Scalar(0, 255, 0), 1);
      }
      if (!skip_pixel_display)
      {
        session_visualizer->display_pixels(std::make_shared < std::vector < std::shared_ptr < cv::Point2f>>>(pixels));
      }
      else
      {
        session_visualizer->display_image(RGB_matrix);
      }
    }
    sleep(1);
  }
}

int main(int argc, char** argv)
{
  std::shared_ptr<fulfil::depthcam::DeviceManager> manager = std::make_shared<fulfil::depthcam::DeviceManager>();

  char hostname[255];
  memset(hostname, 0, sizeof(hostname));
  gethostname(hostname, sizeof(hostname));
  std::cout << "hostname is: " << hostname << std::endl;

  std::shared_ptr<INIReader> reader = std::make_shared<INIReader>("AGX_specific_main.ini", true);
  std::shared_ptr <fulfil::depthcam::Session> current_session;

  // If arg for LFB or Tray provided, stream that video using the provided tray recipe (2.0, 2.1, or 3.1)
  if (argc == 2 && std::string(argv[1]) == "LFB") {
    current_session = manager->session_by_serial_number(reader->Get("device_specific", "LFB_cam"));
    display_session(current_session, "LFB", "2.0");
  }
  else if (argc == 3 && std::string(argv[1]) == "Tray" && ( (std::string(argv[2]) == "2.0") or (std::string(argv[2]) == "2.1") or (std::string(argv[2]) == "3.1"))) {
    std::string tray_type = argv[2];
    current_session = manager->session_by_serial_number(reader->Get("device_specific", "tray_cam"));
    display_session(current_session, "Tray", tray_type);
  }
  else if (argc == 3 && std::string(argv[1]) == "Induction" && ( (std::string(argv[2]) == "2.0") or (std::string(argv[2]) == "2.1") or (std::string(argv[2]) == "3.1"))) {
      std::string tray_type = argv[2];
      current_session = manager->session_by_serial_number(reader->Get("device_specific", "induction_cam"));
      display_session(current_session, "Induction", tray_type);
  }
  else{
    std::cout << "Proper input to the program is 'LFB' or 'Tray/Induction + Tray Generation (2.0, 2.1, or 3.1)'. "
                 "Example: 'Tray 3.1' or 'Induction 2.1'. Please try again.\n";
  }
  return 0;
}