/**
 * This file contains the implementation of class that
 * processes the image to find the aruco markers contained
 * within it and save markers to a file.
 */
#include<memory>
#include <Fulfil.DepthCam/aruco/marker_detector.h>
#include"Fulfil.DepthCam/aruco/marker.h"
#include <Fulfil.CPPUtils/point_3d.h>
#include<string>
#include<opencv2/opencv.hpp>
#include<opencv2/aruco.hpp>
#include<opencv2/highgui.hpp>
#include <Fulfil.DepthCam/core.h>
#include <Fulfil.CPPUtils/logging.h>


using fulfil::utils::Point3D;
using fulfil::depthcam::aruco::MarkerDetector;
using fulfil::depthcam::Session;
using fulfil::depthcam::aruco::Marker;
using fulfil::utils::Logger;

MarkerDetector::MarkerDetector(int nMarkers, int markerSize)
{
    //This parameter allows for the possibility of flexibility in the future if
    //this is desired to be used for more markers.
    this->dictionary = cv::aruco::generateCustomDictionary(nMarkers,markerSize);

}

std::shared_ptr<std::vector<std::shared_ptr<Marker>>> MarkerDetector::detect_markers(std::shared_ptr<cv::Mat> image)
{
  Logger::Instance()->Trace("Detecting markers now");
  std::vector<int> ids;
  std::vector<std::vector<cv::Point2f>> corners, rejected;
  cv::Ptr<cv::aruco::DetectorParameters> detectorParams = cv::aruco::DetectorParameters::create();
  //The corners are in pixels and will be translated later.
  cv::aruco::detectMarkers(*image, this->dictionary, corners, ids, detectorParams, rejected);
  std::shared_ptr<std::vector<std::shared_ptr<Marker>>> detected_markers =
      std::make_shared<std::vector<std::shared_ptr<Marker>>>();

  for(int i = 0; i < ids.size(); i++)
  {
    Logger::Instance()->Trace("Marker detected: {}", ids[i]);
    std::shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>> shared_corners =
        std::make_shared<std::vector<std::shared_ptr<cv::Point2f>>>();
    for(int j = 0; j < corners[i].size(); j++)
    {
      shared_corners->push_back(std::make_shared<cv::Point2f>(corners[i][j].x, corners[i][j].y));
    }
    std::shared_ptr<Marker> marker = std::make_shared<Marker>(ids[i], shared_corners);
    detected_markers->push_back(marker);
  }
  Logger::Instance()->Debug("Detected number of markers is: {}", detected_markers->size());
  return detected_markers;
}

void MarkerDetector::generate_markers(std::string directory_name, int number_of_markers)
{

    // Define how you would like to create the aruco marker with associated white border
    // The border will have pixels_per_mark number of pixels on each side
    int pixels_per_mark = 100;
    int number_squares = 4;

    for(int i = 0; i < number_of_markers; i++)
    {
      cv::Mat image;
      cv::aruco::drawMarker(this->dictionary, i, pixels_per_mark * number_squares, image);

      // let border on each side be the same number of pixels as each pixel on the marker
      int border = pixels_per_mark;
      // constructs a larger image to fit both the image and the border
      cv::Mat border_img;

      copyMakeBorder(image, border_img, border, border,
                     border, border, CV_HAL_BORDER_CONSTANT, (255,255,255));

      std::string output_file = directory_name;
      output_file += std::to_string(i);
      output_file += ".jpg";
      std::cout << output_file << std::endl;

      cv::imwrite(output_file, border_img);
      //free(tmp);
    }
}