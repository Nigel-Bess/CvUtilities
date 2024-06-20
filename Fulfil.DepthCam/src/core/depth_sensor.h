//
// Created by nkaffine on 12/2/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file outlines the functionality of a realsense depth sensor
 * that is used inside of the depth session. It wraps around
 * a lot of the functionality provided by the realsense sdk.
 */
#ifndef FULFIL_DEPTHCAM_DEPTH_SENSOR_H_
#define FULFIL_DEPTHCAM_DEPTH_SENSOR_H_
#include<memory>
#include<chrono>
#include <Fulfil.CPPUtils/timer.h>
#include<eigen3/Eigen/Dense>
#include <Fulfil.CPPUtils/comm/depthCams.pb.h>
#include <Fulfil.CPPUtils/comm/TaskQueue.h>
#include<opencv2/opencv.hpp>
#include<librealsense2/rs.hpp>
using namespace std::chrono;
using namespace fulfil::utils::timing;
namespace fulfil
{
namespace depthcam
{
class DepthSensor
{
 private:
    /**
     * Realsense pipeline object which is used to get rgb and depth data from the sensor.
     */
    std::shared_ptr<rs2::pipeline> pipeline = std::shared_ptr<rs2::pipeline>(new rs2::pipeline());

    /**
     * Realsense decimation object which is used to aggregate depth data to increase
     * accuracy and reduce pixel density in point cloud.
     */
    std::shared_ptr<rs2::decimation_filter> decimation_filter;
    /**
     * Realsense point cloud object to take depth data and create a point cloud for it.
     * Point cloud causes memory leaks so it is only created once per depth sensor in the lifetime of the
     * program and stored.
     */
    std::shared_ptr<rs2::pointcloud> pointcloud;

    std::shared_ptr<rs2::frameset> frame_set;

    std::chrono::system_clock::time_point last_frame_time;
    std::chrono::system_clock::time_point print_time;
    void manage_pipe();
    void create_camera_status_msg(DcCameraStatusCodes code);

    inline bool frame_is_good(){
        return ms_elapsed(last_frame_time) < 500;
    }

    std::mutex _lock;

 public:
  /**
   * Initializes the depth sensor with the given serial number.
   * Note:
   *  An error will be thrown if the given serial number does not correspond
   *  to a device that is connected but it will take a long time to throw the error.
   *  It is recommended that you use the device manager to instantiate the depth
   *  sensor as it performs a check before calling the initializer to check that there
   *  is a connected device with the serial number.
   *
   * @param serial number the serial number of the desired device.
   */
  DepthSensor(const std::string &serial_number);
    rs2::frameset get_latest_frame();
  /**
   * Returns a matrix of points that are in the depth sensor's coordinate system
   * for the given set of frames.
   *
   * @param frameset the set of frames that will be used to create the matrix of points.
   *
   * @return a 3xN matrix of points in the depth sensor's coordinate system (in meters).
   */
  std::shared_ptr<Eigen::Matrix3Xd> get_point_cloud(std::shared_ptr<rs2::depth_frame> raw_depth_frame, std::shared_ptr<rs2::video_frame> raw_color_frame, bool include_invalid_depth_data);

  /**
   * Gets the current frameset from the sensor
   * returns a pair: the unaligned depth frame, and the color+depth frameset (aligned if align_frames = true, unaligned otherwise)
   */
  void get_frameset(bool align_frames, std::shared_ptr<rs2::depth_frame> *raw_depth_frame,
                    std::shared_ptr<rs2::depth_frame> *aligned_depth_frame, std::shared_ptr<rs2::video_frame> *raw_color_frame);


    void get_fresh_frameset(cv::Mat& raw_color_mat);
    void get_fresh_frameset(cv::Mat& raw_color_mat, cv::Mat& raw_depth_mat);
    void get_fresh_frameset(cv::Mat& raw_color_mat, cv::Mat& raw_depth_mat, cv::Mat& aligned_depth_mat);

    /**
     * Get the depth scale of stream (multiple by Z16 value to get distance in meters)
     */
    float get_depth_scale();

        /**
     * Realsense profile object which is used in the process of setting up the pipeline.
     */
    std::shared_ptr<rs2::pipeline_profile> profile;
    /**
     * The intrinsics object from the color stream of the depth sensor.
     */
    std::shared_ptr<rs2_intrinsics> color_intrinsics;

    std::shared_ptr<rs2_intrinsics> depth_intrinsics;

    /**
     * The extrinsics that contain the transormation from the
     * color stream point system to the depth stream point system
     * of the depth sensor.
     */
    std::shared_ptr<rs2_extrinsics> color_to_depth_extrinsics;

    /**
     * The extrinsics that contain the tranformation from the depth
     * stream point system to the color stream point system of the depth
     * sensor.
     */
    std::shared_ptr<rs2_extrinsics> depth_to_color_extrinsics;

    /**
     * The serial number of the depth sensor
     */
    std::shared_ptr<std::string> serial_number;

    /**
     * The pixel width of both the color stream and the
     * depth stream of the depth sensor.
     */
    const int frame_width = 1280;

    /**
     * The pixel height of both the color stream and
     * the depth stream of teh depth sensor.
     */
    const int frame_height = 720;

    uint64_t total_frames = 0;
    uint64_t good_frames = 0;
    uint64_t std_exceptions = 0;
    uint64_t unrecoverable_exc = 0;
    uint64_t recoverable_exc = 0;

    double average_frame_time = 0;

    std::string name_ = "D";

    bool connected_ = false;
};
} // namespace fulfil
} // namespace depthcam
#endif
