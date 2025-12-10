//
// Created by nkaffine on 12/2/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains the implementation of the depth session
 * and the interactions with the depth sensor.
 */
#include <Fulfil.CPPUtils/file_system_util.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/commands/dc_api_error_codes.h>
#include <Fulfil.DepthCam/core/depth_sensor.h>
#include <Fulfil.DepthCam/core/depth_session.h>
#include <Fulfil.DepthCam/point_cloud/no_translation_point_cloud.h>
#include <Fulfil.DepthCam/point_cloud/translated_point_cloud.h>
#include <eigen3/Eigen/Dense>
#include <librealsense2/rs.hpp>
#include <memory>
#include <stdexcept>
#include <cstdio>

using fulfil::utils::FileSystemUtil;
using fulfil::depthcam::DepthSession;
using fulfil::depthcam::pointcloud::NoTranslationPointCloud;
using fulfil::depthcam::pointcloud::TranslatedPointCloud;
using fulfil::utils::Logger;
using fulfil::utils::commands::dc_api_error_codes::DcApiError;
using fulfil::utils::commands::dc_api_error_codes::DcApiErrorCode;

/**
 auto try_set_emitter_control = [&sensor](int desired_setting) {
   if (desired_setting == 0) { return 0 ; }
   auto depth_sensor = sensor->profile->get_device().first<rs2::depth_sensor>();
   if (!depth_sensor.supports(RS2_OPTION_EMITTER_ENABLED)) {
     Logger::Instance()->Warn("Sensor {} does not support emitter control. Disabling emitter settings.", depth_sensor.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER ));
     return 0;
   }
   Logger::Instance()->Info("Sensor {} session emitter control set to {}.", depth_sensor.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER ), desired_setting);
   return desired_setting;
};

 * */


DepthSession::DepthSession(std::shared_ptr<DepthSensor> sensor)
{
    this->sensor = sensor;
    this->raw_depth_frame = nullptr;
    this->aligned_depth_frame = nullptr;
    this->raw_color_frame = nullptr;
    this->emitter_control = 1;      // 0 = always OFF, 1 = always ON, 2 = toggle  TODO: can move to configs or separate functionality by camera if desired
}

std::shared_ptr<rs2_intrinsics> DepthSession::get_color_stream_intrinsics()
{
    return this->sensor->color_intrinsics;
}

cv::Mat DepthSession::grab_color_frame(){
    cv::Mat raw_color_mat; // cv::Mat aligned_depth_mat;
    sensor->get_fresh_frameset(raw_color_mat);
    return raw_color_mat ;
}

void DepthSession::set_sensor_name(const std::string &name){
    sensor->name_ = name;
}

void DepthSession::set_service(std::shared_ptr<GrpcService> serv){
    sensor->service_ = serv;
}

std::shared_ptr<rs2_intrinsics> DepthSession::get_depth_stream_intrinsics()
{
  return this->sensor->depth_intrinsics;
}

std::shared_ptr<rs2_extrinsics> DepthSession::get_color_to_depth_extrinsics()
{
    return this->sensor->color_to_depth_extrinsics;
}

std::shared_ptr<rs2_extrinsics> DepthSession::get_depth_to_color_extrinsics()
{
    return this->sensor->depth_to_color_extrinsics;
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> DepthSession::get_point_cloud(bool include_invalid_depth_data, const char* caller)
{
  Logger::Instance()->Info("Get Point Cloud #1 called in Depth_session by {}", caller);
  return std::make_shared<NoTranslationPointCloud>(sensor->get_point_cloud(this->raw_depth_frame, this->raw_color_frame, include_invalid_depth_data, __FUNCTION__),
                                                this->get_color_stream_intrinsics(),
                                              this->get_depth_stream_intrinsics(),
                                              this->get_color_to_depth_extrinsics(),
                                                this->get_depth_to_color_extrinsics());
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> DepthSession::get_point_cloud(std::shared_ptr<Eigen::Matrix3Xd> rotation,
                                                                            std::shared_ptr<Eigen::Vector3d> translation,
                                                                            bool include_invalid_depth_data,
                                                                            const char* caller)
{
  Logger::Instance()->Info("Get Point Cloud #2 called in Depth_session by {}", caller);
  std::shared_ptr<Eigen::Affine3d> transform = std::make_shared<Eigen::Affine3d>();
  (*transform).linear() = *rotation;
  (*transform).translation() = *translation;
  return this->get_point_cloud(transform, include_invalid_depth_data, __FUNCTION__);
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> DepthSession::get_point_cloud(
    std::shared_ptr<Eigen::Affine3d> transform, bool include_invalid_depth_data, const char* caller)
{
  Logger::Instance()->Info("Get Point Cloud #3 called in Depth_session by {}", caller);
  std::shared_ptr<Eigen::Matrix3Xd> point_matrix = sensor->get_point_cloud(this->raw_depth_frame, this->raw_color_frame, include_invalid_depth_data, __FUNCTION__);

  //apply transform to point cloud
  std::shared_ptr<Eigen::Matrix3Xd> new_point_matrix = std::make_shared<Eigen::Matrix3Xd>(((*transform) * (*point_matrix)));

  return std::make_shared<TranslatedPointCloud>(
      new_point_matrix, transform, this->get_color_stream_intrinsics(), this->get_depth_stream_intrinsics(),
      this->get_color_to_depth_extrinsics(), this->get_depth_to_color_extrinsics());
}

bool DepthSession::check_for_same_frame(cv::Mat color_image, cv::Mat depth_image)
{
  bool function_result = false;

  if(this->cached_RGB_image.empty() or this->cached_depth_image.empty())
  {
    Logger::Instance()->Debug("Refresh same frame comparison is not necessary, no cached image");
  }
  else
  {
    Logger::Instance()->Debug("Validating that refresh resulted in new image now");
    cv::Mat diff1 = color_image != this->cached_RGB_image;
    cv::Mat diff2 = depth_image != this->cached_depth_image;

    int result_1 = cv::countNonZero(diff1);
    int result_2 = cv::countNonZero(diff2);

    Logger::Instance()->Debug("Difference in RGB image from cache: {}", result_1);
    Logger::Instance()->Debug("Difference in Depth image from cache: {}", result_2);

    if(result_1 == 0 and result_2 == 0)
    {
      Logger::Instance()->Error("Both refreshed images are not different from previous cached images; Vars: camera_serial_number = {}", *(this->sensor->serial_number));
      function_result = true;
    }
    else if (result_1 == 0)
    {
      Logger::Instance()->Error("Refreshed RGB image alone is not different from previous cached image; Vars: camera_serial_number = {}", *(this->sensor->serial_number));
      function_result = true;
    }
    else if (result_2 == 0)
    {
      Logger::Instance()->Error("Refreshed depth image alone is not different from previous cached image; Vars: camera_serial_number = {}", *(this->sensor->serial_number));
      function_result = true;
    }
  }

  this->cached_RGB_image = color_image;
  this->cached_depth_image = depth_image;
  return function_result;
}

void DepthSession::get_sensor_frameset(bool align_frames)
{
  sensor->get_frameset(align_frames, &this->raw_depth_frame, &this->aligned_depth_frame, &this->raw_color_frame);
}

void DepthSession::refresh(bool align_frames, bool validate_frames, bool num_retries)
{
  // wait the duration of the frame rate + 10ms for good luck
  int time_ms_to_new_frame = (int)1000/this->sensor->get_frame_rate_in_seconds() + 10;
  for (int i = 0; i < num_retries; i++) {
    try {
      this->get_sensor_frameset(align_frames);
      break;
    }
    catch (const std::exception & e)
    {      
      Logger::Instance()->Error("Refreshing frameset failed on attempt number (0-indexed) {} with error message: {}{}", i, e.what(), (i+1 < num_retries) ? std::string("Waiting ") + std::to_string(time_ms_to_new_frame) + std::string("ms before retrying...") : "");
      std::this_thread::sleep_for(std::chrono::milliseconds(time_ms_to_new_frame));
    }
    catch (...) {
      Logger::Instance()->Error("Refreshing frameset failed on attempt number (0-indexed) {} with unspecified error{}", i, (i+1 < num_retries) ? std::string("Waiting ") + std::to_string(time_ms_to_new_frame) + std::string("ms before retrying...") : "");
      std::this_thread::sleep_for(std::chrono::milliseconds(time_ms_to_new_frame));
    }
  }

  // Check for frame number comparison to detect frozen camera
  if (raw_depth_frame && raw_color_frame)
  {
    uint64_t current_depth_frame_number = raw_depth_frame->get_frame_number();
    uint64_t current_color_frame_number = raw_color_frame->get_frame_number();
    
    // Check if frames are identical to previous frames
    bool depth_frame_same = (current_depth_frame_number == previous_depth_frame_number);
    bool color_frame_same = (current_color_frame_number == previous_color_frame_number);
    
    if (depth_frame_same || color_frame_same)
    {
      // Get current timestamp for timeout check
      auto current_time = std::chrono::steady_clock::now();
      
      // Initialize frozen_frame_start_time if this is the first time we detect frozen frames
      if (!frozen_frame_detected)
      {
        frozen_frame_start_time = current_time;
        frozen_frame_detected = true;
      }
      
      // Check if camera has been frozen for more than the configured timeout
      auto frozen_duration = std::chrono::duration_cast<std::chrono::seconds>(current_time - frozen_frame_start_time);
      if (frozen_duration.count() > FROZEN_CAMERA_TIMEOUT_SECONDS)
      {
        std::string error_msg = "Camera frozen for more than " + std::to_string(FROZEN_CAMERA_TIMEOUT_SECONDS) + " seconds. ";
        if (depth_frame_same && color_frame_same)
        {
          error_msg += "Both depth and color frames are stuck";
        }
        else if (depth_frame_same)
        {
          error_msg += "Depth frame stuck";
        }
        else 
        {
          error_msg += "Color frame stuck";
        }
        error_msg += ". Camera serial: " + *(this->sensor->serial_number);
        
        Logger::Instance()->Error(error_msg);
        throw DcApiError(DcApiErrorCode::FrozenCameraStream, error_msg);
      }
    }
    else
    {
      // Frames have changed, reset frozen detection
      frozen_frame_detected = false;
    }
    
    // Update previous frame numbers for next comparison
    previous_depth_frame_number = current_depth_frame_number;
    previous_color_frame_number = current_color_frame_number;
  }
}

bool DepthSession::set_emitter(bool state)
{
  // TODO: Swap to constexpr free since we set and cant change behavior post start up, and move to sensor.
  //  Possibly change toggle to emitter on/off paired with emitter enabled / disabled

  if (emitter_control == 1 and state == 0) return false; //ignore OFF commands if emitter_control is in Always ON state
  if (emitter_control == 0 and state == 1) return false; //ignore ON commands if emitter_control is in Always OFF state

  rs2::device selected_device = (*this->sensor->profile).get_device();
  if (selected_device.get_info(RS2_CAMERA_INFO_NAME) == std::string_view("Intel RealSense D457")) {
      fulfil::utils::Logger::Instance()->Warn("Set emitter attempted on {}. RS D457 devices have a firmware issue that causes faults during emitter interactions. "
        "Session will not set emitter until Intel resolves issue.", *this->get_serial_number());
      return false;
  }


  auto depth_sensor = selected_device.first<rs2::depth_sensor>();
  if (depth_sensor.supports(RS2_OPTION_EMITTER_ENABLED)) {
    if (state == 1) {
      auto current_state = depth_sensor.get_option(RS2_OPTION_EMITTER_ENABLED);
      if (current_state == 1) {
        Logger::Instance()->Debug("Camera {} emitter state is already ON", *this->get_serial_number());
        return true;
      }
      Logger::Instance()->Debug("Camera {} emitter state is OFF, turning ON and waiting 200ms", *this->get_serial_number());
      depth_sensor.set_option(RS2_OPTION_EMITTER_ENABLED, 1.f);// Enable emitter
      usleep(200000);// time to allow for the emitter to actually turn on after state is set
      return true;
    }
    Logger::Instance()->Debug("Setting camera {} emitter to OFF state", *this->get_serial_number());
    depth_sensor.set_option(RS2_OPTION_EMITTER_ENABLED, 0.F);// Disable emitter
    return true;
  }
  Logger::Instance()->Error("Sensor {} does not support emitter control", *(this->get_serial_number()));
  return false;
}

std::shared_ptr<cv::Mat> DepthSession::get_color_mat()
{
  if (!this->raw_color_frame)
  {
    Logger::Instance()->Error("Raw_Color_Frame is nullptr in get_color_mat method. Exiting Program");
    exit(1);
  }
  Logger::Instance()->Debug("Extracting color frame from Depth Session!");
  // This does NOT create a deep copy. The data is not copied.
  return std::make_shared<cv::Mat>(
          cv::Size(this->sensor->frame_width, this->sensor->frame_height),
          CV_8UC3, (void*) this->raw_color_frame->get_data());
}


std::shared_ptr<cv::Mat> DepthSession::get_depth_mat(bool aligned_frames)
{
  if (!this->aligned_depth_frame or !this->raw_depth_frame)
  {
    // TODO I don't like this, but it is possible that there is something that will break in bag cam if I attempt to refresh here
    Logger::Instance()->Error("One of the depth frames is nullptr in get_depth_mat method. Exiting Program");
    exit(1);
  }
  if (aligned_frames){
      return std::make_shared<cv::Mat>(
        cv::Size(this->sensor->frame_width, this->sensor->frame_height),
        CV_16UC1, (void*) this->aligned_depth_frame->get_data());
  }
    // This does NOT create a deep copy. The data is not copied.
  return std::make_shared<cv::Mat>(
          cv::Size(this->sensor->frame_width, this->sensor->frame_height),
          CV_16UC1, (void*) this->raw_depth_frame->get_data());
}

std::shared_ptr<std::string> DepthSession::get_serial_number()
{
    return this->sensor->serial_number;
}

void DepthSession::lock()
{
    this->session_mutex.lock();
}

void DepthSession::unlock()
{
    this->session_mutex.unlock();
}

float DepthSession::depth_at_pixel(int x, int y)
{
  return this->aligned_depth_frame->get_distance(x, y);
}




