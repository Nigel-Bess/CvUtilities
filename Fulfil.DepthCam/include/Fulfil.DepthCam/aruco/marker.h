/**
 * This file outlines the functionality and data stored on
 * markers that are detected from aruco tags with opencv.
 */
#ifndef FULFIL_DEPTHCAM_MARKER_H_
#define FULFIL_DEPTHCAM_MARKER_H_
#include<memory>
#include <Fulfil.CPPUtils/point_3d.h>
#include<iostream>
#include<stdio.h>
#include <vector>
#include <opencv2/opencv.hpp>

namespace fulfil
{
namespace depthcam
{
namespace aruco
{
class Marker
{
 private:
  int id;
  std::shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>> corners;
 public:
  /**
   * This enum represents a location on an aruco marker. When detected
   * in opencv, the aruco markers provide the four corners of the marker,
   * this enum allows the specification of which point on the marker
   * should be returned for the pixel location.
   */
  enum Coordinate {
    ///The pixel coordinate at the top left of the detected aruco marker.
    topLeft,
    ///The pixel coordinate at the bottom left of the detected aruco marker.
    bottomLeft,
    ///The pixel coordinate at the top right of the detected aruco marker.
    topRight,
    ///The pixel coordinate at the bottom right of the detected aruco marker.
    bottomRight,
    ///The pixel coordinate at the center of the top side of the aruco marker.
    topCenter,
    ///The pixel coordinate at the center of the bottom side of the aruco marker.
    bottomCenter,
    ///The pixel coordinate at the center of the right side of the aruco marker.
    rightCenter,
    ///The pixel coordinate at the center of the left side of the aruco marker.
    leftCenter,
    ///The pixel coordinate at the center of the aruco marker.
    center};
  Marker(int id, std::shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>> corners);
  /**
   * Returns the id of the marker detected by OpenCV
   * @return int id
   */
  int get_id();
  /**
   * Returns the pixel coordinate of the marker at the coordinate described
   * by the enum value.
   * @param coordinate the location of the pixel to be returned relative to the marker.
   * @return a pointer to the OpenCV pixel of the coordinate.
   */
  std::shared_ptr<cv::Point2f> get_coordinate(Coordinate coordinate);
};
} // namespace fulfil
} // namespace depthcam
} // namespace aruco

#endif