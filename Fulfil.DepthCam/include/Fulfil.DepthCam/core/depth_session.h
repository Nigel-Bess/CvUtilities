/**
 * This file outlines the depth session class which implements the abstract
 * session class. It uses the library private sensor class to implement the
 * functionality specified by the abstract session class. The main purpose of
 * making depth session the only way to interact with sensors is to make it
 * easy to cache the last used frame of the sensor and perform multiple operations
 * on the same frame.
 */
#ifndef FULFIL_DEPTHCAM_DEPTH_SESSION_H_
#define FULFIL_DEPTHCAM_DEPTH_SESSION_H_

#include "../../../src/core/depth_sensor.h"

#include<memory>
#include<mutex>

#include <Fulfil.DepthCam/core/session.h>

namespace fulfil
{
namespace depthcam
{
class DepthSession : public Session
{
 private:

  /**
   * The underlying sensor of the session.
   */
  std::shared_ptr<DepthSensor> sensor;
  /**
   * The three different frames that can be accessed for processing
   */
  std::shared_ptr<rs2::depth_frame> raw_depth_frame;

  std::shared_ptr<rs2::depth_frame> aligned_depth_frame;

  std::shared_ptr<rs2::video_frame> raw_color_frame;

  // variable that defines how emitter control is handled.  0 = always on, 1 = always off, 2 = toggle
  int emitter_control;

  /**
   * Mutex for multi-threading with the sensor.
   */
  std::mutex session_mutex;

  /**
   * Cached images to be used during refresh validation to check if frame has indeed updated
   */
  cv::Mat cached_RGB_image;
  cv::Mat cached_depth_image;

  // returns true if one of these two images matches what has been cached from the previous refresh
  bool check_for_same_frame(cv::Mat color_image, cv::Mat depth_image);

 public:
   /**
    * Constructor that takes in a depth sensor that will be stored
    * in the session.
    * @param sensor a point to the depth sensor
    */
  explicit DepthSession(std::shared_ptr<DepthSensor> sensor);

  void lock() override;

  void unlock() override;

  void set_sensor_name(const std::string &name) override;

  std::shared_ptr<std::string> get_serial_number() override;

  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> get_point_cloud(bool include_invalid_depth_data) override;

  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> get_point_cloud(std::shared_ptr<Eigen::Matrix3Xd> rotation,
                                                    std::shared_ptr<Eigen::Vector3d> translation,
                                                    bool include_invalid_depth_data) override;

  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> get_point_cloud(std::shared_ptr<Eigen::Affine3d> transform,
                                                                bool include_invalid_depth_data) override;

  std::shared_ptr<rs2_intrinsics> get_color_stream_intrinsics() override;

  std::shared_ptr<rs2_intrinsics> get_depth_stream_intrinsics() override;

  std::shared_ptr<rs2_extrinsics> get_color_to_depth_extrinsics() override;

  std::shared_ptr<rs2_extrinsics> get_depth_to_color_extrinsics() override;

    void set_service(std::shared_ptr<GrpcService> serv);

  template <size_t N>
  std::array<cv::Mat, N> get_fresh_frameset() {
      static_assert( (N > 0 && N < 4), "get_frameset must be called with a valid N in range 2 to 3!");
      if constexpr (N == 3) {
          cv::Mat raw_color_mat; cv::Mat raw_depth_mat; cv::Mat aligned_depth_mat;
          sensor->get_fresh_frameset(raw_color_mat, raw_depth_mat, aligned_depth_mat);
          return { raw_color_mat, raw_depth_mat, aligned_depth_mat };
      } else if constexpr (N == 2) {
          cv::Mat raw_color_mat; cv::Mat raw_depth_mat; // cv::Mat aligned_depth_mat;
          sensor->get_fresh_frameset(raw_color_mat, raw_depth_mat);
          //cv::Mat aligned_depth_mat = raw_depth_mat; // not a deep copy!
          return { raw_color_mat, raw_depth_mat };
      } else if constexpr (N == 1) {
          cv::Mat raw_color_mat; // cv::Mat aligned_depth_mat;
          sensor->get_fresh_frameset(raw_color_mat);
          //cv::Mat aligned_depth_mat = raw_depth_mat; // not a deep copy!
          return { raw_color_mat };
      }
  }

    cv::Mat grab_color_frame();

  void refresh(bool align_frames = true, bool validate_frames = true) override;

  // state false = turn off emitter,  state true = turn on emitter
  bool set_emitter(bool state) override;

  // Gives a *shallow* copy of held color frame data, in the form of a matrix.
  std::shared_ptr<cv::Mat> get_color_mat() override;

  // Gives a *shallow* copy of held depth frame data, in the form of a matrix.
  std::shared_ptr<cv::Mat> get_depth_mat(bool aligned_frames = true) override;

  float depth_at_pixel(int x, int y) override;
};
}  // namespace core
}  // namespace fulfil

#endif  // FULFIL_DEPTHCAM_DEPTH_SESSION_H_
