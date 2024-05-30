/**
 * This file outlines the functionality for a container
 * that dynamically calculates the transformation to convert
 * points from the camera coordinates into local coordinates
 * using aruco markers and opencv.
 */
#ifndef FULFIL_DEPTHCAM_MARKER_DETECTOR_CONTAINER_H
#define FULFIL_DEPTHCAM_MARKER_DETECTOR_CONTAINER_H

#include <string>
#include <memory>

#include <eigen3/Eigen/Geometry>
#include <Fulfil.DepthCam/aruco/marker_detector.h>
#include <Fulfil.DepthCam/aruco/container.h>
#include <Fulfil.DepthCam/core/session.h>
#include <Fulfil.DepthCam/point_cloud/point_cloud.h>
#include <Fulfil.DepthCam/aruco.h>

namespace fulfil
{
namespace depthcam
{
namespace aruco
{
class MarkerDetectorContainer : public Container
{
 private:

  std::shared_ptr<fulfil::depthcam::Session> session;
  std::shared_ptr<fulfil::depthcam::aruco::FixedTransformContainer> cached_container;
  std::shared_ptr<std::vector<std::shared_ptr<Marker>>> cached_markers = nullptr;

  bool should_filter_points_outside_of_container;
  bool extend_region_over_markers;
  int aruco_retries;
  void setup_cached_container();
  std::shared_ptr<std::vector<Marker::Coordinate>> corners;
  std::shared_ptr<Eigen::Matrix3Xd> mm_marker_coordinates;

  int num_markers;
  float marker_depth;
  float marker_depth_tolerance;
  int min_marker_count_for_validation;

  std::shared_ptr<Eigen::Affine3d> get_transform_to_bag_coordinates(std::shared_ptr<std::vector<std::shared_ptr<Marker>>> detected_markers);
 public:
  cv::Mat grab_color_frame() override;

  int region_min_x;
  int region_max_x;
  int region_min_y;
  int region_max_y;

  std::shared_ptr<MarkerDetector> marker_detector;
   /**
    * MarkerDetectorContainer Constructor.
    * @param marker_detector the marker detector that is used to detect the aruco markers in the rgb image
    * of the provided session.
    * @param session the session that will be used for the underlying data.
    * @param should_filter_points_outside_of_container containers create a bounding box based
    * on the rectangle formed by the aruco markers detected, width, length, and vector of corners.
    * If this flag is true, points that are outside of the bounds of the container will be excluded
    * from point clouds returned by this object. If the flag is false, all points, including those
    * that are outside of the container, will be included in all point clouds returned by this
    * project.
    *
    * @param extend_region_over_markers does absolutely nothing at the moment.
    * This boolean used to extend the local depth cloud in the container to also inspect over the
    * aruco markers on the LFR bot. This was intended for inspecting items that may be sticking out of the container
    * and overflowing the edges of the bot. For now, a True value will lead to an error log and no extension.
    * WARNING: this functionality is not currently in use in the production code. It should be restructured before used
    * WARNING: caching containers with this extension method is dangerous as it is easy to forget that the region is
    * extended. This is important especially for things like pre and post dispense comparison algorithms via cached containers.
    * Or comparisons of online processing to offline saved data.
    *
    * @param width the width (parallel to the x-axis) of the container.
    * @param length the length (parallel to the y-axis) of the container.
    * @param corners a vector of the marker feature locations. The id of the marker it corresponds to is the index in the vector.
    * @param mm_marker_coordinates stores the local coordinate system coordinates for each marker in mm
    * @param marker_depth is the depth in camera coordinates of the markers that are being detected
    * @param num_markers is the expected number of standard markers to detect on the LFB
    * @param region_ inputs define the bounds of the search region for the markers. Defined in pixel space of the RGB image (default 1280 x 720 pixels for full image)
    */
   MarkerDetectorContainer(std::shared_ptr<MarkerDetector> marker_detector,
                           std::shared_ptr<Session> session,
                           bool should_filter_points_outside_of_container,
                           bool extend_region_over_markers,
                           float width,
                           float length,
                           float outer_width,
                           float outer_length,
                           std::shared_ptr<std::vector<Marker::Coordinate>> corners,
                           std::shared_ptr<Eigen::Matrix3Xd> mm_marker_coordinates,
                           int num_markers,
                           float marker_depth,
                           float marker_depth_tolerance,
                           int min_marker_count_for_validation,
                           int region_max_x,
                           int region_min_x,
                           int region_max_y,
                           int region_min_y);
  /**
   * Returns the transform found from the aruco markers.
   * @return
   */
   /**
    * Returns the transformation found to convert camera coordinates to container coordinates.
    * @return a pointer to the transformation converting camera coordinates to container coordinates.
    * @throws invalid_argument_exception when there aren't enough corners detected after the
    * given number of retries.
    */
  std::shared_ptr<Eigen::Affine3d> get_transform();

  void lock() override;

  void unlock() override;

  std::shared_ptr<std::string> get_serial_number() override;

  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> get_point_cloud(
      bool include_invalid_depth_data) override;

  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> get_point_cloud(
      std::shared_ptr<Eigen::Matrix3Xd> rotation,
      std::shared_ptr<Eigen::Vector3d> translation,
      bool include_invalid_depth_data) override;

  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> get_point_cloud(
      std::shared_ptr<Eigen::Affine3d> transform,
      bool include_invalid_depth_data) override;

  void refresh(bool align_frames = true, bool validate_frames = true) override;

  bool set_emitter(bool state) override;

  std::shared_ptr<cv::Mat> get_color_mat() override;

  std::shared_ptr<cv::Mat> get_depth_mat(bool aligned_frames = true) override;

  /**
   * This function returns the depth data in the local coordinate system at a certain pixel location
   */
  float depth_at_pixel(int x, int y) override;


  /**
  * This function returns the depth data in the camera coordinate system at a certain pixel location
  */
  float camera_depth_at_pixel(int x, int y);

  std::shared_ptr<rs2_intrinsics> get_color_stream_intrinsics() override;

  std::shared_ptr<rs2_intrinsics> get_depth_stream_intrinsics() override;

  std::shared_ptr<rs2_extrinsics> get_color_to_depth_extrinsics() override;

  std::shared_ptr<rs2_extrinsics> get_depth_to_color_extrinsics() override;

  /** validates that detected markers were within acceptable search region and at expected depth
   *
   * @param markers is the vector of markers detected in the complete RGB image
   * @return vector of markers with valid coordinates
   */
  std::shared_ptr<std::vector<std::shared_ptr<Marker>>> validate_markers(std::shared_ptr<std::vector<std::shared_ptr<Marker>>> markers, bool bot_in_nominal_orientation=true);

  std::shared_ptr<std::vector<std::shared_ptr<Marker>>> get_markers(bool bot_in_nominal_orientation=true);

  /**
   * A static convenience variable that returns a vector that can be used to
   * use the center of the aruco markers as the pixel coordinate for markers.
   * @return a point to a vector of 8 coordinates that are all center.
   */
  static std::shared_ptr<std::vector<Marker::Coordinate>> all_centers()
  {
    std::shared_ptr<std::vector<Marker::Coordinate>> corners = std::make_shared<std::vector<Marker::Coordinate>>();
    for(int i = 0; i < 8; i++)
    {
      corners->push_back(Marker::Coordinate::center);
    }
    return corners;
  }

  /**
 * A static convenience variable that returns a vector that can be used to
 * use different parts of the aruco markers for definitions. See depth cam diagrams for more details
 * @return a point to a vector of 8 coordinates, where the corner at index i corresponds to the relevant marker location for marker id i
 */
  static std::shared_ptr<std::vector<Marker::Coordinate>> centers_and_sides(int marker_count)
  {
    std::shared_ptr<std::vector<Marker::Coordinate>> corners = std::make_shared<std::vector<Marker::Coordinate>>();
    for(int i = 0; i < marker_count; i++)
    {
      if(i == 1 or i == 2)
      {
        corners->push_back(Marker::Coordinate::topCenter);
      }
      else if(i == 0 or i == 7)
      {
        corners->push_back(Marker::Coordinate::leftCenter);
      }
      else
      {
        corners->push_back(Marker::Coordinate::center);
      }
    }
    return corners;
  }

};
}  // namespace aruco
}  // namespace core
}  // namespace fulfil


#endif
