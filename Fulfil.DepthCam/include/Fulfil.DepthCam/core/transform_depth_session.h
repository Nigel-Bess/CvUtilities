/**
 * The purpose of this file is to provide an outline of the functionality
 * for a session where the points are transformed by a fixed transformation.
 * This is helpful when you have a known transformation between the coordinates
 * of the underlying session and want to work with the transformed points.
 */
#ifndef FULFIL_DEPTHCAM_TRANSFORM_DEPTH_SESSION_H
#define FULFIL_DEPTHCAM_TRANSFORM_DEPTH_SESSION_H

#include <Fulfil.DepthCam/core/session.h>

#include <string>
#include <memory>

#include <opencv2/opencv.hpp>

namespace fulfil
{
namespace depthcam
{
class TransformDepthSession : public fulfil::depthcam::Session
{
 private:
  std::shared_ptr<Session> session;
  std::shared_ptr<Eigen::Affine3d> transform;
  std::shared_ptr<cv::Mat> transformed_distance_mat;
  float invalid_distance_data_value;
  int stream_x_max;
  int stream_y_max;
  int x_max;
  int y_max;
  int x_min;
  int y_min;
  std::vector<cv::Point2i> samples;
  //std::shared_ptr<cv::Mat> get_color_mat_depth_threshold(float depth_clipping_distance);
  std::shared_ptr<cv::Mat> build_local_distance_mat();
  bool point_in_spatial_extents(int x, int y);
  void get_sample_point(cv::Point2i& sample);
  bool can_reuse_distance_mat();

public:
  /**
   * Constructor for the transform depth session. Takes in
   * the base session and the transformation that will be applied
   * to the coordinate system from the base session.
   * @param session the session where the original point clouds will
   * be taken from.
   * @param transform the transformation that will be applied to
   * the session.
   */
  TransformDepthSession(std::shared_ptr<Session> session,
                        std::shared_ptr<Eigen::Affine3d> transform,
                        int x_min, int x_max, int y_min, int y_max,
                        float invalid_distance_data_value);


  void lock() override;

  void unlock() override;

    cv::Mat grab_color_frame() override;


    std::shared_ptr<std::string> get_serial_number() override;

  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> get_point_cloud(bool include_invalid_depth_data) override;

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

  std::shared_ptr<cv::Mat> get_local_distance_mat();

  float depth_at_pixel(int x, int y) override;

  std::shared_ptr<rs2_intrinsics> get_color_stream_intrinsics() override;

  std::shared_ptr<rs2_intrinsics> get_depth_stream_intrinsics() override;

  cv::Point2i get_pixel_from_point(float x, float y, float z);

  std::shared_ptr<Eigen::Vector3d> get_point_from_pixel(int x, int y);

   std::shared_ptr<rs2_extrinsics> get_color_to_depth_extrinsics();

   std::shared_ptr<rs2_extrinsics> get_depth_to_color_extrinsics();
   void set_sensor_name(const std::string &name) override;
   void set_service(std::shared_ptr<GrpcService> serv) override;
};
}
}

#endif // FULFIL_DEPTHCAM_TRANSFORM_DEPTH_SESSION_H
