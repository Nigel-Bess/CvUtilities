//
// Created by steve on 5/18/20.
//

#include <Fulfil.Dispense/drop.h>
#include <Fulfil.DepthCam/point_cloud.h>
#include <memory>
#include <Fulfil.DepthCam/visualization.h>
#include <iostream>
#include <unistd.h>
#include <Fulfil.CPPUtils/logging.h>

using fulfil::depthcam::visualization::SessionVisualizer;
using fulfil::depthcam::pointcloud::LocalPointCloud;
using fulfil::depthcam::pointcloud::PixelPointCloud;
using fulfil::depthcam::pointcloud::PointCloud;
using fulfil::utils::Logger;

int visual_number = 1;

void onMouse_calibrate(int event, int x, int y, int flags, void* param)
{
  if (event == cv::EVENT_LBUTTONDBLCLK)
  {
    if (visual_number < 7)
    {
      visual_number += 1;
      std::cout << "Changed visual to number: " << visual_number << std::endl;
    }
    else
    {
      visual_number = 1;
      std::cout << "Changed visual to number: " << visual_number << std::endl;
    }
  }
}

void open_viewer(std::string base_path, int position, std::string window_name)
{
  std::cout << "Open Viewer function called" << std::endl;
  std::string image_path_1 = base_path;
  image_path_1 = image_path_1.append(std::string("image_1.png"));

  std::string image_path_2 = base_path;
  image_path_2 = image_path_2.append(std::string("image_2.png"));

  std::string image_path_3 = base_path;
  image_path_3 = image_path_3.append(std::string("image_3.png"));

  std::string image_path_4 = base_path;
  image_path_4 = image_path_4.append(std::string("image_4.png"));

  std::string image_path_5 = base_path;
  image_path_5 = image_path_5.append(std::string("image_5.png"));

  std::string image_path_6 = base_path;
  image_path_6 = image_path_6.append(std::string("image_6.png"));

  std::string image_path_7 = base_path;
  image_path_7 = image_path_7.append(std::string("image_7.png"));



  /**
   *  User defined window location for the viewer
   */
  std::shared_ptr<std::pair <int, int>> window_size = std::make_shared<std::pair <int, int>>(960,540); // 960, 540   //1280,  720 for one monitor,  960, 540  for laptop
  std::pair <int, int> window_location;

  if (position < 0 || position > 4)
  {
    std::cout << "Incorrect position variable, please fix" << std::endl;
    exit(1);
  }

  if (position == 0) window_location = std::pair <int, int>(0,0);
  if (position == 1) window_location = std::pair <int, int>(1000,0);
  if (position == 2) window_location = std::pair <int, int>(0,720);
  if (position == 3) window_location = std::pair <int, int>(1000,720);


  int num_iterations = 0;
  std::shared_ptr<cv::Mat> RGB_matrix;

  std::cout << "Starting viewer now" << std::endl;
  while(num_iterations < 1000000)
  {
    switch(visual_number)
    {
      case 1:
      {
        RGB_matrix = std::make_shared<cv::Mat>(cv::imread(image_path_1, cv::IMREAD_COLOR));
        break;
      }
      case 2:
      {
        RGB_matrix = std::make_shared<cv::Mat>(cv::imread(image_path_2, cv::IMREAD_COLOR));
        break;
      }
      case 3:
      {
        RGB_matrix = std::make_shared<cv::Mat>(cv::imread(image_path_3, cv::IMREAD_COLOR));
        break;
      }
      case 4:
      {
        RGB_matrix = std::make_shared<cv::Mat>(cv::imread(image_path_4, cv::IMREAD_COLOR));
        break;
      }
      case 5:
      {
        RGB_matrix = std::make_shared<cv::Mat>(cv::imread(image_path_5, cv::IMREAD_COLOR));
        break;
      }
      case 6:
      {
        RGB_matrix = std::make_shared<cv::Mat>(cv::imread(image_path_6, cv::IMREAD_COLOR));
        break;
      }
      case 7:
      {
        RGB_matrix = std::make_shared<cv::Mat>(cv::imread(image_path_7, cv::IMREAD_COLOR));
        break;
      }
    }

    if (RGB_matrix->empty()) //if first image isn't present, don't show any
    {
      cv::destroyAllWindows();
      std::cout << "No file at live viewer location at the moment... waiting for 1 seconds" << std::endl;
      cv::namedWindow(window_name.c_str(), cv::WINDOW_NORMAL);
      cv::setMouseCallback(window_name.c_str(), onMouse_calibrate, 0);
      cv::resizeWindow(window_name.c_str(), window_size->first,window_size->second );
      cv::moveWindow(window_name.c_str(), window_location.first, window_location.second);
      cv::waitKey(1000);
      sleep(1);
    }
    else
    {
      cv::namedWindow(window_name.c_str(), cv::WINDOW_NORMAL);
      cv::setMouseCallback(window_name.c_str(), onMouse_calibrate, 0);
      cv::imshow(window_name.c_str(), *RGB_matrix);
      cv::resizeWindow(window_name.c_str(), window_size->first,window_size->second);
      cv::moveWindow(window_name.c_str(), window_location.first,  window_location.second);
      cv::waitKey(1);

      sleep(1);
    }
    num_iterations += 1;
  }

}

int main(int argc, char** argv)
{
  Logger* logger = Logger::Instance(Logger::default_logging_dir, log_file_name, Logger::Level::Info,Logger::Level::Debug);

    // Get configs
  std::shared_ptr<INIReader> reader = std::make_shared<INIReader>("main_config.ini", true);
  if (reader->ParseError() < 0) {
    logger->Fatal("Can't load main_config.ini, check path.");
    throw std::runtime_error("Failure to parse ini file...");
  }

  int viewer;
  std::cout << "Which viewer would you like to open:  Drop Viewer = 0,  Tray Viewer = 1 ?  ";
  std::cin >> viewer;

  int position;
  std::cout << "Position for the viewer?  Top-Left = 0, Top-Right = 1, Bottom-Left - 2, Bottom-Right = 3 ?   ";
  std::cin >> position;

  int num_viewer;
  std::cout << "Which number viewer is this?   ";
  std::cin >> num_viewer;

  std::string image_file_path;

  if (viewer == 0)
  {
    std::cout << "You have chosen to open a viewer for Drop camera" << std::endl;
    std::string filename = reader->Get("device_specific", "live_visualize_drop_path", "Couldn't Find Path, check INI config file");
    //std::string filename = reader->Get("steve_local_testing", "live_visualize_drop_path", "Couldn't Find Path, check INI config file");    //For offline testing
    std::cout << "Images will be displayed from path:" << filename << std::endl;

    std::string filename2 = filename;

    open_viewer(filename, position, std::string("Live_Viewer_").append(std::to_string(num_viewer)));
  }
  else if (viewer == 1)
  {
    std::cout << "You have chosen to open a viewer for Tray camera" << std::endl;
    std::string filename = reader->Get("device_specific", "live_visualize_tray_path", "Couldn't Find Path, check INI config file");
    //std::string filename = reader->Get("steve_local_testing", "live_visualize_drop_path", "Couldn't Find Path, check INI config file");    //For offline testing
    std::cout << "Images will be displayed from path:" << filename << std::endl;
    open_viewer(filename, 0, std::string("Live Viewer 1"));
  }
  else
  {
    std::cout << "Please enter an integer corresponding to the live viewer you would like to open" << std::endl;
  }

  cv::destroyAllWindows();
  return 0;
}