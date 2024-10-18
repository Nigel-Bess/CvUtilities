/**
 * This file outlines the functionality for detecting aruco tags
 * from the image frame from a session.
 */
#ifndef FULFIL_DEPTHCAM_MARKER_DETECTOR_H_
#define FULFIL_DEPTHCAM_MARKER_DETECTOR_H_

#include"marker.h"

#include<vector>
#include <memory>

#include<librealsense2/rs.hpp>
#include<opencv2/opencv.hpp>
#include<opencv2/aruco.hpp>
#include <Fulfil.DepthCam/core.h>

namespace fulfil
{
namespace depthcam
{
namespace aruco
{
class MarkerDetector
{
 public:
  /**
   * Constructor for marker detector. Takes no arguments and initializes with
   * a hard coded aruco dictionary.
   */
  MarkerDetector(int nMarkers, int markerSize);

  /**
   * The dictionary used to determine which aruco markers will be detected.
   */
  cv::Ptr<cv::aruco::Dictionary> dictionary;

  /**
   * Return a vector of the marker details that are detected from the given session.
   * @param rgb image that will be used to find aruco markers

   * @return a pointer to a vector of markers which contain the details of the
   * detected aruco markers.
   */
  std::shared_ptr<std::vector<std::shared_ptr<Marker>>> detect_markers(std::shared_ptr<cv::Mat> image);

  /**
   * Creates a folder in the given directory with the images of the markers in the
   * dictionary of this object.
   *
   * @param directory_path the directory to save the folder with all of the images.
   *
   * @param number_of_markers the number of markers to be generated
   */
   /**
    * Creates a folder in the given directory with the images of the markers in the
    * dictionary stored in this object.
    * @param directory_path the path to the directory where the images will be stored.
    * @param number_of_markers the number of markers to be created. (unknown behavior
    * when this number exceeds the number of markers in the dictionary).
    */
  void generate_markers(std::string directory_path, int number_of_markers);
};
}  // namespace aruco
}  // namespace core
}  // namespace fulfil

#endif