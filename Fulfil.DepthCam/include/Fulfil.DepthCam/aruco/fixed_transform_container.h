/**
 * This file outlines the functionality for a container that
 * has a fixed transform stored in the object to convert
 * from camera coordinates to the desired coordinate system.
 */
#ifndef FULFIL_DEPTHCAM_FIXED_TRANSFORM_CONTAINER_H
#define FULFIL_DEPTHCAM_FIXED_TRANSFORM_CONTAINER_H

#include <memory>
#include <string>

#include <eigen3/Eigen/Geometry>
#include <Fulfil.DepthCam/aruco/container.h>
#include <Fulfil.DepthCam/core/transform_depth_session.h>

namespace fulfil
{
namespace depthcam
{
namespace aruco
{
using MatrixPoint =
Eigen::Block<Eigen::Matrix<float, 3, -1, 0, 3, -1>, 3, 1, true>;

class FixedTransformContainer : public Container
{
 private:
  std::shared_ptr<Eigen::Affine3d> transform;
  std::shared_ptr<Session> session;
  bool should_filter_points_outside_of_container;
 public:
  /**
   * Constructor for fixed transform container.
   * @param session the session that will be used for initial point clouds.
   * @param transform the transformation that will be applied to the
   * data retrieved from the internal session.
   * @param should_filter_points_outside_of_container containers create a
   * bounding box from the width and length where the center is (0,0).
   * If this flag is true, any point that is outside of the bounding
   * box will not be included in the point clouds retrieved from this
   * session. If this flag is false, all points, including the points
   * outside of the bounding box, will be included in point clouds
   * retrieved from this session.
   * @param width the width (side parallel to the x-axis) of the container
   * in the coordinate system after applying the transformation.
   * @param length the length (side parallel to the y-axis) of the container
   * in the coordinate system after applying the transformation.
   */
  FixedTransformContainer(std::shared_ptr<Session> session,
                          std::shared_ptr<Eigen::Affine3d> transform,
                          bool should_filter_points_outside_of_container,
                          float width, float length, float center_x = 0, float center_y = 0, float min_depth = -100, float max_depth = 100);

  /**
   * The size of the edge that is parallel with the x-axis of the bag,
   * in the local coordinate system. (initially set to zero to
   * catch errors from not setting the variable)
   */
  float width = 0;
  /**
   * The size of the edge that is parallel with the y-axis of the bag,
   * in the local coordinate system. (initially set to zero to catch
   * errors from not setting the variable)
   */
  float length = 0;

  /**
   *   X local coordinate of the center of the container matrix. X axis is in line with width
   */
  float center_x;
  /**
   *  Y local coordinate of the center of the container matrix. Y axis is in line with length
   */
  float center_y;

  /**
   *   Minimum local coordinate depth allowed for filtering
   */
  float min_depth;
  /**
   *   Maximum local coordinate depth allowed for filtering
   */
  float max_depth;

   /**
    * Returns the transform that is applied to all of the point clouds
    * to convert them to the local coordinate system.
    * @return a pointer to the transformation matrix used to convert from
    * camera coordinates to the local coordinate system.
    */
  std::shared_ptr<Eigen::Affine3d> get_transform();

  void lock() override;

  void unlock() override;

    cv::Mat grab_color_frame() override;

  std::shared_ptr<std::string> get_serial_number() override;

  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> get_point_cloud(
      bool include_invalid_depth_data, const char* caller) override;

  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> get_point_cloud(
      std::shared_ptr<Eigen::Matrix3Xd> rotation,
      std::shared_ptr<Eigen::Vector3d> translation,
      bool include_invalid_depth_data, const char* caller) override;

  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> get_point_cloud(
      std::shared_ptr<Eigen::Affine3d> transform,
      bool include_invalid_depth_data, const char* caller) override;

  void refresh(bool align_frames = true, bool validate_frames = true, bool num_retries = 3) override;

  bool set_emitter(bool state) override;

  std::shared_ptr<cv::Mat> get_color_mat() override;

  std::shared_ptr<cv::Mat> get_depth_mat(bool aligned_frames = true) override;

  std::shared_ptr<rs2_intrinsics> get_color_stream_intrinsics() override;

  std::shared_ptr<rs2_intrinsics> get_depth_stream_intrinsics() override;

  std::shared_ptr<rs2_extrinsics> get_color_to_depth_extrinsics() override;

  std::shared_ptr<rs2_extrinsics> get_depth_to_color_extrinsics() override;

  float depth_at_pixel(int x, int y) override;

  void set_service(std::shared_ptr<GrpcService> serv) override;
};
}  // namespace aruco
}  // namespace core
}  // namespace fulfil

#endif
