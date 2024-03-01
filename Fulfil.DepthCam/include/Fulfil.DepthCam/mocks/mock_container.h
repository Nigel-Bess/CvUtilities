//
// Created by nkaffine on 12/2/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * The purpose of this file is to create a way to mock containers using mock
 * data that has been saved. This outlines the additional logic required
 * to optionally filter out points that are outside of the bounds of the
 * container.
 */
#ifndef FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_MOCKS_MOCK_CONTAINER_H_
#define FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_MOCKS_MOCK_CONTAINER_H_
#include <Fulfil.DepthCam/aruco.h>
#include <memory>
#include <Fulfil.DepthCam/mocks/mock_session.h>

namespace fulfil
{
namespace depthcam
{
namespace mocks
{
class MockContainer : public fulfil::depthcam::aruco::Container
{
 private:
  bool should_filter_points_outside_of_container;
  std::shared_ptr<fulfil::depthcam::Session> session;
 public:
  /**
   * MockContainer constructor.
   * @param session the session that is used for the mock container to get
   * the depth camera data.
   * @param should_filter_points_outside_of_container the mock container creates
   * a bounding box from the width and length. If this flag is true, points that
   * fall outside of that point box will not be included in point clouds returned
   * from this object. If the flag is false, points that fall outside of the
   * bounding box will be included in point clouds returned from this object.
   * @param width the width (parallel to the x-axis) of the container in
   * the local coordinate system of the session.
   * @param length the length (parallel to the y-axis) of the container in
   * the local coordinate system of the session.
   */
  MockContainer(std::shared_ptr<fulfil::depthcam::Session> session,
                bool should_filter_points_outside_of_container,
                float width, float length);

  void lock() override;

  void unlock() override;
    cv::Mat grab_color_frame() override;


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

  float depth_at_pixel(int x, int y) override;
};
} // namespace fulfil
} // namespace core
} // namespace mocks

#endif //FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_MOCKS_MOCK_CONTAINER_H_
