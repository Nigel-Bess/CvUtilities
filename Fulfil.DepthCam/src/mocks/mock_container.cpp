//
// Created by nkaffine on 12/2/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains the implementation of a mock container which
 * takes in a mock session and specifies the size of the container
 * represented by that mock session.
 */
#include <Fulfil.DepthCam/mocks/mock_container.h>
#include <Fulfil.CPPUtils/eigen.h>
#include "../aruco/container_matrix3xd_predicate.h"
#include <Fulfil.DepthCam/point_cloud.h>

using fulfil::depthcam::mocks::MockContainer;
using fulfil::utils::eigen::Matrix3dPredicate;
using fulfil::utils::eigen::Matrix3XdFilter;
using fulfil::depthcam::aruco::ContainerMatrix3xdPredicate;
using std::make_shared;

MockContainer::MockContainer(std::shared_ptr<fulfil::depthcam::Session> session,
                             bool should_filter_points_outside_of_container,
                             float width,
                             float length)
{
  this->should_filter_points_outside_of_container = should_filter_points_outside_of_container;
  this->width = width;
  this->length = length;
  this->session = session;
}

void MockContainer::lock()
{
  this->session->lock();
}

void MockContainer::unlock()
{
  this->session->unlock();
}

void MockContainer::set_service(std::shared_ptr<GrpcService> serv){
    this->session->set_service(serv);
}

cv::Mat MockContainer::grab_color_frame() {
    return this->session->grab_color_frame();
}

std::shared_ptr<std::string> MockContainer::get_serial_number()
{
  return this->session->get_serial_number();
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud>
    MockContainer::get_point_cloud(bool include_invalid_depth_data)
{
  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> full_point_cloud =
      this->session->get_point_cloud(include_invalid_depth_data);
  if(this->should_filter_points_outside_of_container)
  {
    std::shared_ptr<fulfil::depthcam::pointcloud::LocalPointCloud> local_point_cloud = full_point_cloud->as_local_cloud();
    std::shared_ptr<Matrix3dPredicate> predicate =
        make_shared<ContainerMatrix3xdPredicate>(this->width, this->length);
    std::shared_ptr<Matrix3XdFilter> filter = make_shared<Matrix3XdFilter>(predicate);
    local_point_cloud->apply_filter(filter);
    return local_point_cloud;
  }
  else
  {
    return full_point_cloud;
  }
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> MockContainer::get_point_cloud(std::shared_ptr<Eigen::Matrix3Xd> rotation,
                                                                                         std::shared_ptr<Eigen::Vector3d> translation,
                                                                                         bool include_invalid_depth_data)
{
  std::shared_ptr<Eigen::Affine3d> transform = std::shared_ptr<Eigen::Affine3d>(new Eigen::Affine3d());
  (*transform).linear() = *rotation;
  (*transform).translation() = *translation;
  return this->get_point_cloud(transform, include_invalid_depth_data);
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> MockContainer::get_point_cloud(std::shared_ptr<Eigen::Affine3d> transform,
                                                                                         bool include_invalid_depth_data)
{
  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud = this->get_point_cloud(include_invalid_depth_data);
  point_cloud->add_transformation(transform);
  return point_cloud;
}

void MockContainer::refresh(bool align_frames, bool validate_frames, bool num_retries)
{
  this->session->refresh();
}

std::shared_ptr<cv::Mat> MockContainer::get_color_mat()
{
  return this->session->get_color_mat();
}

std::shared_ptr<cv::Mat> MockContainer::get_depth_mat(bool aligned_frames)
{
  return this->session->get_depth_mat();
}

float MockContainer::depth_at_pixel(int x, int y)
{
  return this->session->depth_at_pixel(x, y);
}