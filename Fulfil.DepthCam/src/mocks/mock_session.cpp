//
// Created by nkaffine on 12/2/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains the implementation of a mock session which
 * reads the data required for a session from a directory and
 * allows it to be treated as a regular session.
 */
#include <Fulfil.CPPUtils/eigen.h>
#include <Fulfil.CPPUtils/file_system_util.h>
#include <Fulfil.CPPUtils/logging.h>
#include "Fulfil.DepthCam/coders/intrinsics_coder.h"
#include "Fulfil.DepthCam/coders/extrinsics_coder.h"
#include "Fulfil.DepthCam/coders/matrix3xd_coder.h"
#include "Fulfil.DepthCam/mocks/mock_session.h"
#include <Fulfil.DepthCam/point_cloud/no_translation_point_cloud.h>
#include <Fulfil.DepthCam/point_cloud/untranslated_point_cloud.h>
#include <Fulfil.DepthCam/point_cloud/camera_point_cloud.h>
#include "Fulfil.DepthCam/point_cloud/point_cloud_decoder.h"

#include <memory>
#include <random>

using fulfil::utils::FileSystemUtil;
using fulfil::depthcam::mocks::MockSession;
using fulfil::depthcam::pointcloud::NoTranslationPointCloud;
using fulfil::depthcam::pointcloud::UntranslatedPointCloud;
using fulfil::depthcam::pointcloud::CameraPointCloud;
using fulfil::depthcam::pointcloud::Matrix3xdCoder;
using fulfil::depthcam::pointcloud::PointCloudDecoder;
using fulfil::depthcam::pointcloud::ExtrinsicsCoder;
using fulfil::depthcam::pointcloud::IntrinsicsCoder;
using fulfil::utils::eigen::CustomMatrix3dPredicate;
using fulfil::utils::eigen::Matrix3XdFilter;
using fulfil::utils::eigen::Matrix3dPoint;
using std::string;
using std::shared_ptr;
using std::vector;
using std::invalid_argument;
using std::make_shared;
using std::to_string;
using std::runtime_error;
using fulfil::utils::Logger;


void MockSession::populate_mock_session_members(shared_ptr<std::string> path)
{
  this->directory_path = path;

  //Populate the aligned_depth_image
  std::string filename = *this->directory_path;
  FileSystemUtil::join_append(filename, "aligned_depth_image.png");
  if(!FileSystemUtil::file_exists(filename.c_str()))
  {
    Logger::Instance()->Error("No aligned depth image available!");
    throw invalid_argument("no aligned depth image provided");
  }
  cv::Mat mat = cv::imread(filename, CV_16UC1);
  this->aligned_depth_frame = make_shared<cv::Mat>(mat);

  //Populate the raw_depth_image
  filename = *this->directory_path;
  FileSystemUtil::join_append(filename, "raw_depth_image.png");
  if(!FileSystemUtil::file_exists(filename.c_str()))
  {
    Logger::Instance()->Error("No raw depth image available at {}!", filename.c_str());
    throw invalid_argument("no raw depth image provided");
  }
  mat = cv::imread(filename, CV_16UC1);
  this->raw_depth_frame = make_shared<cv::Mat>(mat);

  //Populate the raw_color_image
  filename = *this->directory_path;
  FileSystemUtil::join_append(filename, "color_image.png");
  if(!FileSystemUtil::file_exists(filename.c_str()))
  {
    Logger::Instance()->Error("No color image available at {}!", filename.c_str());
    throw invalid_argument("no color image provided");
  }
  mat = cv::imread(filename);
  this->raw_color_frame = make_shared<cv::Mat>(mat);

  //populate color intrinsics
  std::shared_ptr<std::string> color_intrinsics_filename = std::make_shared<std::string>();
  color_intrinsics_filename->append(*this->directory_path);
  color_intrinsics_filename->append("/color_intrinsics");
  this->color_stream_intrinsics = IntrinsicsCoder::decode_from_file(color_intrinsics_filename);

  //populate depth intrinsics
  std::shared_ptr<std::string> depth_intrinsics_filename = std::make_shared<std::string>();
  depth_intrinsics_filename->append(*this->directory_path);
  depth_intrinsics_filename->append("/depth_intrinsics");
  this->depth_stream_intrinsics = IntrinsicsCoder::decode_from_file(depth_intrinsics_filename);

  std::shared_ptr<std::string> extrinsics_depth_to_color_filename = std::make_shared<std::string>();
  extrinsics_depth_to_color_filename->append(*this->directory_path);
  extrinsics_depth_to_color_filename->append("/depth_to_color_extrinsics");
  this->depth_to_color_extrinsics = ExtrinsicsCoder::decode_from_file(extrinsics_depth_to_color_filename);

  std::shared_ptr<std::string> extrinsics_color_to_depth_filename = std::make_shared<std::string>();
  extrinsics_color_to_depth_filename->append(*this->directory_path);
  extrinsics_color_to_depth_filename->append("/color_to_depth_extrinsics");
  this->color_to_depth_extrinsics = ExtrinsicsCoder::decode_from_file(extrinsics_color_to_depth_filename);

  //populate point cloud
  this->point_cloud =
      PointCloudDecoder::decode_from_directory(this->directory_path);
}

MockSession::MockSession(shared_ptr<std::string> directory_path)
{
    this->populate_mock_session_members(directory_path);
}

MockSession::MockSession(shared_ptr<std::string> directory_path, const string& mock_serial)
{
  this->serial_number = make_shared<string>(mock_serial);
  this->populate_mock_session_members(directory_path);
}

MockSession::MockSession(shared_ptr<std::string> directory_path, const shared_ptr<std::string>& mock_serial)
{
  this->serial_number = mock_serial;
  this->populate_mock_session_members(directory_path);
}

MockSession::MockSession(std::shared_ptr<fulfil::depthcam::Session> session)
{
  this->raw_depth_frame = nullptr;

  cv::Mat depth_img = session->get_depth_mat(true)->clone();
  this->aligned_depth_frame = std::make_shared<cv::Mat>(depth_img);

  cv::Mat color_img = session->get_color_mat()->clone();
  this->raw_color_frame = std::make_shared<cv::Mat>(color_img);

  //Todo: this point cloud needs to be re-created from data so it is not still tied to the original capture frame?? Maybe not
  this->point_cloud = session->get_point_cloud(true, __FUNCTION__);

  this->serial_number = nullptr;
  this->color_stream_intrinsics = session->get_color_stream_intrinsics();
  this->depth_stream_intrinsics = session->get_depth_stream_intrinsics();
  this->depth_to_color_extrinsics = session->get_depth_to_color_extrinsics();
  this->color_to_depth_extrinsics = session->get_color_to_depth_extrinsics();
  
}

void MockSession::lock()
{
    this->session_mutex.lock();
}

void MockSession::set_service(std::shared_ptr<GrpcService> serv){
    return;
}
void MockSession::unlock()
{
    this->session_mutex.unlock();
}

std::shared_ptr<rs2_intrinsics> MockSession::get_color_stream_intrinsics()
{
  if (this->color_stream_intrinsics == nullptr)
  {
    Logger::Instance()->Error("Intrinsics not available in mock session!");
    throw(16);
  }
  return this->color_stream_intrinsics;
}

std::shared_ptr<rs2_intrinsics> MockSession::get_depth_stream_intrinsics()
{
  if (this->depth_stream_intrinsics == nullptr)
  {
    Logger::Instance()->Error("Intrinsics not available in mock session!");
    throw(16);
  }
  return this->depth_stream_intrinsics;
}


std::shared_ptr<rs2_extrinsics> MockSession::get_color_to_depth_extrinsics()
{
  if (this->color_to_depth_extrinsics == nullptr)
  {
    Logger::Instance()->Error("Intrinsics not available in mock session!");
    throw(16);
  }
  return this->color_to_depth_extrinsics;
}

std::shared_ptr<rs2_extrinsics> MockSession::get_depth_to_color_extrinsics()
{
  if (this->depth_to_color_extrinsics == nullptr)
  {
    Logger::Instance()->Error("Intrinsics not available in mock session!");
    throw(16);
  }
  return this->depth_to_color_extrinsics;
}


std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> MockSession::get_point_cloud(
    bool include_invalid_depth_data, const char* caller)
{
  if (this->point_cloud == nullptr)
  {
    Logger::Instance()->Error("Point cloud not available in mock session!");
    throw(16);
  }

  if(!include_invalid_depth_data)
  {
    shared_ptr<CustomMatrix3dPredicate> predicate = make_shared<CustomMatrix3dPredicate>([](const Matrix3dPoint& point) { return point(2) != 0; });
    shared_ptr<CameraPointCloud> camera_cloud = this->point_cloud->as_camera_cloud();
    camera_cloud->apply_filter(make_shared<Matrix3XdFilter>(predicate));
    return camera_cloud;
  }
  else
  {
    return this->point_cloud;
  }
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> MockSession::get_point_cloud(
    std::shared_ptr<Eigen::Matrix3Xd> rotation,
    std::shared_ptr<Eigen::Vector3d> translation,
    bool include_invalid_depth_data, const char* caller)
{
    std::shared_ptr<Eigen::Affine3d> transform = std::shared_ptr<Eigen::Affine3d>(new Eigen::Affine3d());
    (*transform).linear() = *rotation;
    (*transform).translation() = *translation;
    return this->get_point_cloud(transform, include_invalid_depth_data, __FUNCTION__);
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> MockSession::get_point_cloud(std::shared_ptr<Eigen::Affine3d> transform,
                                                               bool include_invalid_depth_data, const char* caller)
{
  shared_ptr<CameraPointCloud> cam_cloud = this->get_point_cloud(include_invalid_depth_data, __FUNCTION__)->as_camera_cloud();
  return cam_cloud->add_transformation(transform);
}

void MockSession::refresh(bool align_frames, bool validate_frames, bool num_retries)
{
  Logger::Instance()->Warn("Note: frames are not updated during a refresh call for a mock session!");
}

bool MockSession::set_emitter(bool state)
{
  Logger::Instance()->Warn("Note: set emitter does nothing for a mock session!");
  return state;
}

std::shared_ptr<cv::Mat> MockSession::get_color_mat()
{
  if (this->raw_color_frame == nullptr)
  {
    Logger::Instance()->Error("Color frame not available in mock session!");
    throw(16);
  }
  return this->raw_color_frame;
}

cv::Mat MockSession::grab_color_frame() {
    return cv::imread(*this->directory_path +  "/color_image.png");
}

std::shared_ptr<cv::Mat> MockSession::get_depth_mat(bool aligned_frames)
{
  if (this->aligned_depth_frame == nullptr)
  {
    Logger::Instance()->Error("Aligned depth frame not available in mock session!");
    throw(16);
  }
  
  return this->aligned_depth_frame;
}

std::shared_ptr<std::string> MockSession::get_serial_number()
{
  if (this->serial_number) return this->serial_number;
  std::string serial_num;
  std::cout << "Please enter a mock serial number: " << std::endl;
  std::cin >> serial_num;
  std::cout << "Now using serial number "<< serial_num << std::endl;
  *this->serial_number = serial_num;
  return std::make_unique<std::string>(serial_num);
}

float MockSession::depth_at_pixel(int x, int y)
{
  if (this->aligned_depth_frame == nullptr)
  {
    Logger::Instance()->Error("Aligned depth frame not available in mock session, for depth_at_pixel method!");
    throw(16);
  }
  //0.0010 is the depth unit that is used in the presets in the library.
  uint16_t depth_data = this->aligned_depth_frame->at<uint16_t>(y,x);
  return depth_data * 0.001;
}
void fulfil::depthcam::mocks::MockSession::set_sensor_name(const string &name) { this->sensor_name=name ; }
