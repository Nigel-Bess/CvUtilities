/**
 * The purpose of this file is to outline the required functionality
 * for mocking a session using data that is stored from the
 * mock_session_generator class. It specifies functionality of keeping
 * track of available stored frames and samples and current frames
 * and sample.
 */
#ifndef FULFIL_DEPTHCAM_MOCK_SESSION_H
#define FULFIL_DEPTHCAM_MOCK_SESSION_H

#include <memory>
#include <string>
#include <mutex>

#include <Fulfil.DepthCam/core/session.h>

namespace fulfil
{
namespace depthcam
{
namespace mocks
{
class MockSession : public Session
{
 private:
  std::shared_ptr<cv::Mat> raw_depth_frame;
  std::shared_ptr<cv::Mat> aligned_depth_frame;
  std::shared_ptr<cv::Mat> raw_color_frame;

  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud;

  std::shared_ptr<std::string> directory_path;

  std::mutex session_mutex;
  std::shared_ptr<std::string> serial_number = nullptr;
  std::string sensor_name{};

  std::shared_ptr<rs2_intrinsics> color_stream_intrinsics = nullptr;

    std::shared_ptr<rs2_intrinsics> depth_stream_intrinsics = nullptr;

  std::shared_ptr<rs2_extrinsics> color_to_depth_extrinsics = nullptr;

  std::shared_ptr<rs2_extrinsics> depth_to_color_extrinsics = nullptr;

  std::shared_ptr<std::string> get_frame_directory();
  void populate_mock_session_members(std::shared_ptr<std::string> path);
 public:
  /**
   * MockSession constructor
   * @param directory_path the filepath to the directory that contains the data
   * for the mock session.
   */
  explicit MockSession(std::shared_ptr<std::string> directory_path);
  MockSession(std::shared_ptr<std::string> directory_path, const std::string& mock_serial);
  MockSession(std::shared_ptr<std::string> directory_path, const std::shared_ptr<std::string>& mock_serial);

  // constructor for use during online capturing of session data. Used if want to cache current state of session for use somewhere
  MockSession(std::shared_ptr<fulfil::depthcam::Session> session);

    cv::Mat grab_color_frame() override;


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

  float depth_at_pixel(int x, int y) override;

  std::shared_ptr<rs2_intrinsics> get_color_stream_intrinsics() override;

  std::shared_ptr<rs2_intrinsics> get_depth_stream_intrinsics() override;

  std::shared_ptr<rs2_extrinsics> get_color_to_depth_extrinsics() override;

  std::shared_ptr<rs2_extrinsics> get_depth_to_color_extrinsics() override;
  void set_sensor_name(const std::string &name) override;
  void set_service(std::shared_ptr<GrpcService> serv) override;
};
}  // namespace mocks
}  // namespace core
}  // namespace fulfil

#endif
