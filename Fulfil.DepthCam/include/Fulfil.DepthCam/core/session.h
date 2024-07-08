/**
 * The purpose of this file is to outline the required functionality
 * for sessions, which are used throughout this library. It provides
 * requirements for locking and unlocking to work with multi
 * threaded programs,  accessing information about underlying
 * sensors, accessing point cloud data, refreshing the frames being
 * used in the session, exporting the color and depth images, and
 * determining the depth at a given pixel.
 */
#ifndef FULFIL_DEPTHCAM_SESSION_H_
#define FULFIL_DEPTHCAM_SESSION_H_

#include <memory>
#include <string>

#include <librealsense2/rs.hpp>
#include <eigen3/Eigen/Geometry>
#include <opencv2/opencv.hpp>
// #include "../../Fulfil.CPPUtils/include/Fulfil.CPPUtils/comm/depthCams.pb.h"
#include <Fulfil.CPPUtils/comm/GrpcService.h>
#include <Fulfil.DepthCam/point_cloud/point_cloud.h>


namespace fulfil
{
namespace depthcam
{
class Session
{
 public:
  /**
   * Locks the session and all internal components to allow for
   * safe use in multithreaded code.
   */
  virtual void lock() = 0;
  /**
   * Unlocks the session and all internal components to allow
   * other threads to access the resources.
   */
  virtual void unlock() = 0;

  virtual void set_sensor_name(const std::string &name) = 0;
   /**
    * Returns the serial number of the sensor that the session is using.
    * @return a pointer to the serial number string.
    */
  virtual std::shared_ptr<std::string> get_serial_number() = 0;
   /**
    * Returns a point cloud of the data from the session
    * @param include_invalid_depth_data the session have depth values of zero when the
    * session is not able to determine the depth at a given point. If this flag is true, all
    * raw data will be returned including the invalid depth data. If this flag is false,
    * it will return only the points where the session was able to find valid depth data.
    * @return a point cloud of the data from the session.
    */
  virtual std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> get_point_cloud(bool include_invalid_depth_data) = 0;
   /**
    * Gets the a point cloud from the session where the local cloud is the local cloud of the session
    * with the extra step of applying the given transformation.
    * @param rotation 3x3 rotation matrix for the transformation
    * @param translation 3x1 vector translation for the transformation.
    * @param include_invalid_depth_data the sessions have depth values of zero when the
    * session is not able to determine the depth at a given point. If this flag is true, all
    * raw data will be returned including the invalid depth data. If the flag is false,
    * it will return only the points where the session was able to find valid depth data.
    * @return a point cloud of the data from the session where the local cloud has the
    * given transformation details applied.
    */
  virtual std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> get_point_cloud(
      std::shared_ptr<Eigen::Matrix3Xd> rotation,
      std::shared_ptr<Eigen::Vector3d> translation,
      bool include_invalid_depth_data) = 0;
  /**
   * Gets the a point cloud from the session where the local cloud is the local cloud of the session
   * with the extra step of applying the given transformation.
   * @param transform the transformation that will be applied to the point cloud.
   * @param include_invalid_depth_data the sessions have depth values of zero when the
   * session is not able to determine the depth at a given point. If this flag is true, all
   * raw data will be returned including the invalid depth data. If the flag is false,
   * it will return only the points where the session was able to find valid depth data.
   * @return a point cloud of the data from the session where the local cloud has the
    * given transformation applied.
   */
  virtual std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> get_point_cloud(
      std::shared_ptr<Eigen::Affine3d> transform,
      bool include_invalid_depth_data) = 0;
  /**
   * Captures a new set of frame from the session to be used for processing
   * point clouds and images.
   */
  virtual void refresh(bool align_frames = true, bool validate_frames = true) = 0;

  /**
   * Sets the state of the sensor's emitter based on boolean input (1 = enable, 0 = disable)
   */
  virtual bool set_emitter(bool state) = 0;

  /**
   * Returns an OpenCV matrix with the data for the color image stream
   * from the session. The format of the stream is 1280x720 3 channel
   * 8 bit rgb data.
   * @return a pointer to the OpenCV matrix
   */
  virtual std::shared_ptr<cv::Mat> get_color_mat() = 0;
  /**
   * Returns an OpenCV matrix with the data for the depth image stream
   * from the session. The format of the stream is 1280x720 1 channel
   * 16 bit depth data.
   * @return a pointer to the OpenCV matrix.
   */
  virtual std::shared_ptr<cv::Mat> get_depth_mat(bool aligned_frames = true) = 0;
  /**
   * Returns the recorded depth in the local coordinate system of the
   * session at the given pixel coordinate.
   * @param x the x coordinate of the pixel.
   * @param y the y coordinate of the pixel.
   * @return a float depth at the given pixel in the local coordinate
   * system.
   */
  virtual float depth_at_pixel(int x, int y) = 0;

    /**
   * Returns the intrinsics for the color stream of the depth sensor.
   * Note:
   *  The intrinsics are used to convert to and from pixel points from
   *  the color frame to 3D points in the color streams coordinate system.
   */
   virtual std::shared_ptr<rs2_intrinsics> get_color_stream_intrinsics() = 0;

   virtual std::shared_ptr<rs2_intrinsics> get_depth_stream_intrinsics() = 0;

    /**
     * Returns the extrinsics to translate coordinates from the color stream's
     * coordinate system to the depth stream's coordinate system.
     */
    virtual std::shared_ptr<rs2_extrinsics> get_color_to_depth_extrinsics() = 0;
    /**
    * Returns the extrinsics to translate coordaintes from the depth stream's
    * coordinate system to the color stream's coordinate system.
    */
    virtual std::shared_ptr<rs2_extrinsics> get_depth_to_color_extrinsics() = 0;

    virtual cv::Mat grab_color_frame() = 0;
    virtual void set_service(std::shared_ptr<GrpcService> serv) = 0;
};
}  // namespace core
}  // namespace fulfil

#endif