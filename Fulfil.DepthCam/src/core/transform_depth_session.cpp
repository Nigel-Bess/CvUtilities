/**
 * This file contains the implementation of the TransformDepthSession
 * which handles the transformation of points from the camera
 * coordinate system to the desired coordinate system based on the
 * transformation.
 */
#include "Fulfil.DepthCam/core/transform_depth_session.h"
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/timer.h>
#include <Fulfil.DepthCam/point_cloud.h>
#include <librealsense2/h/rs_types.h>
#include <librealsense2/rsutil.h>
#include <stdlib.h>

using fulfil::utils::timing::Timer;
using fulfil::depthcam::TransformDepthSession;
using fulfil::utils::Logger;


TransformDepthSession::TransformDepthSession(std::shared_ptr<Session> session, std::shared_ptr<Eigen::Affine3d> transform,
         int x_min, int x_max, int y_min, int y_max, float invalid_distance_data_value)
{
  this->session = session;
  this->transform = std::make_shared<Eigen::Affine3d>(*transform);
  if (invalid_distance_data_value * 1000 + 10000 > 65535 || invalid_distance_data_value * 1000 + 10000 < 0)
    throw std::runtime_error("Invalid invalid_distance_data_value set. Should be between -10 and 55.535!");
  this->invalid_distance_data_value = invalid_distance_data_value;
  this->stream_x_max = session->get_color_stream_intrinsics()->width;
  this->stream_y_max = session->get_color_stream_intrinsics()->height;
  this->x_max = (x_max == -1) ? this->stream_x_max : x_max;
  this->y_max = (y_max == -1) ? this->stream_y_max : y_max;
  this->x_min = x_min;
  this->y_min = y_min;
  this->samples.resize(8);
  std::for_each(this->samples.begin(), this->samples.end(), [&](cv::Point2i& s) {get_sample_point(s);});

  
}

void TransformDepthSession::get_sample_point(cv::Point2i& sample) {
  int y_range = this->y_max - this->y_min;
  int x_range = this->x_max - this->x_min;
  int y = rand() % y_range + this->y_min;
  int x = rand() % x_range + this->x_min;
  while (session->depth_at_pixel(x, y) == 0) {
    y = rand() % y_range + this->y_min;
    x = rand() % x_range + this->x_min;
  }
  sample.x = x;
  sample.y = y;
}

bool TransformDepthSession::can_reuse_distance_mat(){
  bool same_frame = true;
  if (!this->transformed_distance_mat) return false;
  std::for_each(this->samples.begin(), this->samples.end(), [&](cv::Point2i& s) {
    same_frame = (same_frame && (this->depth_at_pixel(s.x, s.y) == this->transformed_distance_mat->at<float_t>(s)));});
  return same_frame;
}

void TransformDepthSession::lock()
{
    this->session->lock();
}

void TransformDepthSession::unlock()
{
    this->session->unlock();
}

std::shared_ptr<std::string> TransformDepthSession::get_serial_number()
{
    return this->session->get_serial_number();
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> TransformDepthSession::get_point_cloud(
    bool include_invalid_depth_data, const char* caller)
{
    return this->session->get_point_cloud(this->transform, include_invalid_depth_data, caller);
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> TransformDepthSession::get_point_cloud(
    std::shared_ptr<Eigen::Matrix3Xd> rotation,
    std::shared_ptr<Eigen::Vector3d> translation,
    bool include_invalid_depth_data, const char* caller)
{
    std::shared_ptr<Eigen::Affine3d> transformation = std::make_shared<Eigen::Affine3d>();
    (*transformation).linear() = (*rotation);
    (*transformation).translation() = *translation;
    return this->get_point_cloud(transformation, include_invalid_depth_data, caller);
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> TransformDepthSession::get_point_cloud(
    std::shared_ptr<Eigen::Affine3d> transform,
    bool include_invalid_depth_data, const char* caller)
{
    return this->session->get_point_cloud(std::make_shared<Eigen::Affine3d>((*transform) * (*this->transform)),
            include_invalid_depth_data, caller);
}

void TransformDepthSession::refresh(bool align_frames, bool validate_frames, bool num_retries)
{
  this->transformed_distance_mat = nullptr;
  this->session->refresh();

}

std::shared_ptr<cv::Mat> TransformDepthSession::get_color_mat()
{
    return this->session->get_color_mat();
}


std::shared_ptr<rs2_intrinsics> TransformDepthSession::get_color_stream_intrinsics()
{
  return this->session->get_color_stream_intrinsics();
}

std::shared_ptr<rs2_intrinsics> TransformDepthSession::get_depth_stream_intrinsics()
{
  return this->session->get_depth_stream_intrinsics();
}

std::shared_ptr<rs2_extrinsics> TransformDepthSession::get_color_to_depth_extrinsics()
{
  return this->session->get_color_to_depth_extrinsics();
}

std::shared_ptr<rs2_extrinsics> TransformDepthSession::get_depth_to_color_extrinsics()
{
  return this->session->get_depth_to_color_extrinsics();
}

bool TransformDepthSession::set_emitter(bool state)
{
  return this->session->set_emitter(state);
}

//TODO some sort of decoder saved in data gen
std::shared_ptr<cv::Mat> TransformDepthSession::get_depth_mat(bool aligned_frames)
{
  if (aligned_frames){
    if (!this->can_reuse_distance_mat()){
      this->transformed_distance_mat = this->build_local_distance_mat();
      std::for_each(this->samples.begin(), this->samples.end(), [&](cv::Point2i& s) {get_sample_point(s);});
    }
    std::shared_ptr<cv::Mat> depth_image = std::make_shared<cv::Mat>(cv::Size(this->stream_x_max, this->stream_x_max),
                                                                     CV_16UC1, cv::Scalar(0));

    //  10,000 additive factor chosen. 6m is max distance on D455, so should be safe.
    this->transformed_distance_mat->convertTo(*depth_image, CV_16UC1, 1000, 10000);
    depth_image->setTo(0, *depth_image == uint16_t(10000 + 1000*this->invalid_distance_data_value));
    return depth_image;
  }

  return this->session->get_depth_mat(aligned_frames);
}


std::shared_ptr<cv::Mat> TransformDepthSession::get_local_distance_mat()
{
  std::shared_ptr<cv::Mat> local_depth = std::make_shared<cv::Mat>(cv::Size(this->stream_x_max,this->stream_y_max),
                                                                   CV_32FC1);
  if (!this->can_reuse_distance_mat()){
    this->transformed_distance_mat = this->build_local_distance_mat();
    std::for_each(this->samples.begin(), this->samples.end(), [&](cv::Point2i& s) {get_sample_point(s);});
  }
  this->transformed_distance_mat->copyTo(*local_depth);
  return local_depth;
}

float TransformDepthSession::depth_at_pixel(int x, int y)
{
    // TODO think more if this is the way we want to do this.
    return this->get_point_from_pixel(x, y)->z();
}

std::shared_ptr<Eigen::Vector3d> TransformDepthSession::get_point_from_pixel(int x, int y)
{
    auto pixels_distance = session->depth_at_pixel(x, y); // get the camera depth at pixel, frames aligned
    bool invalid_pixel = (pixels_distance == 0);
    if (!invalid_pixel) {
        float color_point[3];
        const float raw_pixel[2] = {static_cast<float>(x), static_cast<float>(y)};
        rs2_deproject_pixel_to_point(color_point, this->session->get_color_stream_intrinsics().get(),
                                     raw_pixel, pixels_distance);
        std::shared_ptr<Eigen::Vector3d> eigen_point = std::make_shared<Eigen::Vector3d>(color_point[0],
                                                                                         color_point[1], color_point[2]);
        if (transform)
            *eigen_point = *transform * (*eigen_point);
        return eigen_point;
    }
    return std::make_shared<Eigen::Vector3d>(0,0,0);
}

cv::Point2i TransformDepthSession::get_pixel_from_point(float x, float y, float z)
{
    Eigen::Vector3d eigen_point = this->transform->inverse() * Eigen::Vector3d(x,y,z);
    float pixel[2];
    float point[3] = {(float)eigen_point(0),(float)eigen_point(1),(float)eigen_point(2)};
    rs2_project_point_to_pixel(pixel, this->session->get_color_stream_intrinsics().get(), point);
    return cv::Point2i(pixel[0], pixel[1]);
}


std::shared_ptr<cv::Mat>
TransformDepthSession::build_local_distance_mat()
{
  std::shared_ptr<cv::Mat> local_depth = std::make_shared<cv::Mat>(cv::Size(this->stream_x_max,this->stream_y_max),
          CV_32FC1, cv::Scalar(this->invalid_distance_data_value));
  Logger::Instance()->Debug("Mask extents: ({}, {}) to ({}, {}), for a total of {} spatial pixels processed. ", x_min, x_max, y_min, y_max,
                            (x_max - x_min)*(y_max - y_min));

  // TODO faster conversion when there is no transform
  for (int y = y_min; y < y_max; y++)
  {
    for (int x = x_min; x < x_max; x++)
    {
      auto pixels_distance = this->session->depth_at_pixel(x, y);
      bool invalid_pixel = (pixels_distance == 0);
      if (this->transform && !invalid_pixel) {
        float color_point[3];
        const float raw_pixel[2] = {static_cast<float>(x), static_cast<float>(y)};
        rs2_deproject_pixel_to_point(color_point, this->session->get_color_stream_intrinsics().get(), raw_pixel, pixels_distance);
        std::shared_ptr<Eigen::Vector3d> eigen_point = std::make_shared<Eigen::Vector3d>(color_point[0],
                                                                                         color_point[1], color_point[2]);
        *eigen_point = *this->transform * (*eigen_point);
        pixels_distance = eigen_point->z();

      }
      // Check if the depth value is invalid (<=0) or greater than the threshold
      local_depth->at<float>(y, x) = pixels_distance;
    }
  }
  Logger::Instance()->Debug("Local Distance Matrix built!!!!!!!!");
  return local_depth;
}


bool TransformDepthSession::point_in_spatial_extents(int x, int y) {
    return (this->x_min<=x) & (this->x_max>x) & (this->y_min<=y) & (this->y_max>y);
}

void fulfil::depthcam::TransformDepthSession::set_sensor_name(const std::string &name) {
    this->session->set_sensor_name(name) ;
}

void TransformDepthSession::set_service(std::shared_ptr<GrpcService> serv){
    this->session->set_service(serv);
}
