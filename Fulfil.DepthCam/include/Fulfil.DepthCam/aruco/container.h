/**
 * The purpose of this file is to outline the functionality
 * for a container which wraps around a session and handles
 * the filtering of points from that session that are not in
 * the conatiner.
 */
#ifndef FULFIL_DEPTHCAM_CONTAINER_H_
#define FULFIL_DEPTHCAM_CONTAINER_H_

#include <memory>
#include <vector>
#include <string>

#include <eigen3/Eigen/Geometry>
#include <Fulfil.DepthCam/core/session.h>
#include <Fulfil.DepthCam/aruco/marker_detector.h>

namespace fulfil
{
namespace depthcam
{
namespace aruco
{
class Container : public Session
{
 public:
  /**
   * The size of the edge that is parallel with the x-axis of the bag. Measured in meters.
   * (initially set to zero to catch errors from not setting the variable)
   */
  float width = 0;
  /**
   * The size of the edge that is parallel with the y-axis of the bag. Measured in meters.
   * (initially set to zero to catch errors from not setting the variable)
   */
  float length = 0;

  float outer_width = 0;
  float outer_length = 0;

  void lock() override = 0;

  void unlock() override = 0;

    cv::Mat grab_color_frame() = 0;

  std::shared_ptr<std::string> get_serial_number() override = 0;

  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> get_point_cloud(
      bool include_invalid_depth_data, const char* caller) override = 0;

  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> get_point_cloud(
      std::shared_ptr<Eigen::Matrix3Xd> rotation,
      std::shared_ptr<Eigen::Vector3d> translation,
      bool include_invalid_depth_data, const char* caller) override = 0;

  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> get_point_cloud(
      std::shared_ptr<Eigen::Affine3d> transform,
      bool include_invalid_depth_data, const char* caller) override = 0;

  void refresh(bool align_frames = true, bool validate_frames = true, bool num_retries = 3) override = 0;

  bool set_emitter(bool state) override = 0;

  std::shared_ptr<cv::Mat> get_color_mat() override = 0;

  std::shared_ptr<cv::Mat> get_depth_mat(bool aligned_frames = true) override = 0;

  std::shared_ptr<rs2_intrinsics> get_color_stream_intrinsics() override = 0;

  std::shared_ptr<rs2_intrinsics> get_depth_stream_intrinsics() override = 0;

  std::shared_ptr<rs2_extrinsics> get_color_to_depth_extrinsics() override = 0;

  std::shared_ptr<rs2_extrinsics> get_depth_to_color_extrinsics() override = 0;
  inline void set_sensor_name(const std::string &name) override {}
};


}  // namespace aruco
}  // namespace core
}  // namespace fulfil

#endif