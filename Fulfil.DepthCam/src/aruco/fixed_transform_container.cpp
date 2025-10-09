//
// Created by nkaffine on 12/2/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains the implementation of a container that has a
 * fixed transform stored in the object.
 */
#include <Fulfil.DepthCam/aruco/fixed_transform_container.h>
#include <Fulfil.DepthCam/core/transform_depth_session.h>
#include "container_matrix3xd_predicate.h"
#include <Fulfil.CPPUtils/eigen/matrix3xd_filter.h>
#include <Fulfil.DepthCam/point_cloud/local_point_cloud.h>
#include <Fulfil.DepthCam/point_cloud.h>
#include <Fulfil.CPPUtils/logging.h>

using fulfil::depthcam::aruco::FixedTransformContainer;
using std::shared_ptr;
using std::make_shared;
using fulfil::utils::eigen::Matrix3dPredicate;
using fulfil::depthcam::aruco::ContainerMatrix3xdPredicate;
using fulfil::utils::eigen::Matrix3XdFilter;
using fulfil::utils::Logger;

FixedTransformContainer::FixedTransformContainer(shared_ptr<Session> session, shared_ptr<Eigen::Affine3d> transform,
                                                 bool should_filter_points_outside_of_container, float width,
                                                 float length, float center_x, float center_y, float min_depth, float max_depth) :
  session {session}, transform {transform}, should_filter_points_outside_of_container {should_filter_points_outside_of_container},
    width {width}, length {length}, center_x {center_x}, center_y {center_y},
      min_depth {min_depth}, max_depth {max_depth} {}

std::shared_ptr<Eigen::Affine3d> FixedTransformContainer::get_transform()
{
    return this->transform;
}

void FixedTransformContainer::lock()
{
    this->session->lock();
}

void FixedTransformContainer::unlock()
{
    this->session->unlock();
}

cv::Mat FixedTransformContainer::grab_color_frame() {
    return this->session->grab_color_frame();
}

std::shared_ptr<std::string> FixedTransformContainer::get_serial_number()
{
    return this->session->get_serial_number();
}

void FixedTransformContainer::set_service(std::shared_ptr<GrpcService> serv){
    this->session->set_service(serv);
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> FixedTransformContainer::get_point_cloud_side_dispense(
    bool include_invalid_depth_data, const char* caller)
{
    std::shared_ptr<fulfil::depthcam::pointcloud::LocalPointCloud> local_point_cloud = this->session->get_point_cloud(
        this->transform, include_invalid_depth_data, caller)->as_local_cloud();

    if (!this->should_filter_points_outside_of_container) { return local_point_cloud; }

    //adjusting the center of the bag to have accurate container mapping
    this->center_x = 0.00;
    std::shared_ptr<Matrix3dPredicate> predicate =
        std::make_shared<ContainerMatrix3xdPredicate>(this->width, this->length, this->center_x, this->center_y, this->min_depth, this->max_depth);
    std::shared_ptr<Matrix3XdFilter> filter = std::make_shared<Matrix3XdFilter>(predicate);
    Logger::Instance()->Debug("Aplying side dispense point cloud filter in FixedTransformContainer!!");
    local_point_cloud->apply_filter_side_dispense(filter);
    return local_point_cloud;
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> FixedTransformContainer::get_point_cloud_side_dispense_outside_cavity(
    bool include_invalid_depth_data, const char* caller)
{
    std::shared_ptr<fulfil::depthcam::pointcloud::LocalPointCloud> local_point_cloud_outside_cavity = this->session->get_point_cloud(
        this->transform, include_invalid_depth_data, caller)->as_local_cloud();

    if (!this->should_filter_points_outside_of_container) { return local_point_cloud_outside_cavity; }

    //adjusting the center of the bag to have accurate container mapping
    this->center_x = 0.00;
    std::shared_ptr<Matrix3dPredicate> predicate =
        std::make_shared<ContainerMatrix3xdPredicate>(this->width, this->length, this->center_x, this->center_y, this->min_depth, this->max_depth);
    std::shared_ptr<Matrix3XdFilter> filter = std::make_shared<Matrix3XdFilter>(predicate);
    Logger::Instance()->Debug("Aplying side dispense outside bag cavity point cloud filter in FixedTransformContainer!!");
    local_point_cloud_outside_cavity->apply_filter_side_dispense_point_cloud_outside_cavity(filter);
    return local_point_cloud_outside_cavity;
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> FixedTransformContainer::get_point_cloud(
    bool include_invalid_depth_data, const char* caller)
{
    //TODO: add some caching
    //std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> full_point_cloud = this->session->get_point_cloud(
      //  this->transform, include_invalid_depth_data);
    std::shared_ptr<fulfil::depthcam::pointcloud::LocalPointCloud> local_point_cloud = this->session->get_point_cloud(
                                                this->transform, include_invalid_depth_data, caller)->as_local_cloud();

    if(!this->should_filter_points_outside_of_container) { return local_point_cloud; }
    //std::shared_ptr<fulfil::depthcam::pointcloud::LocalPointCloud> local_point_cloud = full_point_cloud->as_local_cloud();
    std::shared_ptr<Matrix3dPredicate> predicate =
        std::make_shared<ContainerMatrix3xdPredicate>(this->width, this->length, this->center_x, this->center_y, this->min_depth, this->max_depth);
    std::shared_ptr<Matrix3XdFilter> filter = std::make_shared<Matrix3XdFilter>(predicate);
    Logger::Instance()->Debug("Aplying point cloud filter in FixedTransformContainer!!");
    local_point_cloud->apply_filter(filter);
    return local_point_cloud;
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> FixedTransformContainer::get_point_cloud(
    std::shared_ptr<Eigen::Matrix3Xd> rotation,
    std::shared_ptr<Eigen::Vector3d> translation,
    bool include_invalid_depth_data, const char* caller)
{
  //TODO: think about what to do here about filtering out non container points, right now just not filtering them out.
  return this->session->get_point_cloud(rotation, translation, include_invalid_depth_data, caller);
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> FixedTransformContainer::get_point_cloud(
    std::shared_ptr<Eigen::Affine3d> transform,
    bool include_invalid_depth_data, const char* caller)
{
  //TODO: think about what to do here about filtering out non container points, right now just not filtering them out.
  return this->session->get_point_cloud(transform, include_invalid_depth_data, caller);
}

void FixedTransformContainer::refresh(bool align_frames, bool validate_frames, bool num_retries)
{
    this->session->refresh();
}

bool FixedTransformContainer::set_emitter(bool state)
{
    return this->session->set_emitter(state);
}

std::shared_ptr<cv::Mat> FixedTransformContainer::get_color_mat()
{
    return this->session->get_color_mat();
}

std::shared_ptr<cv::Mat> FixedTransformContainer::get_depth_mat(bool aligned_frames)
{
    return this->session->get_depth_mat(aligned_frames);
}

std::shared_ptr<rs2_intrinsics> FixedTransformContainer::get_color_stream_intrinsics()
{
  return this->session->get_color_stream_intrinsics();
}

std::shared_ptr<rs2_intrinsics> FixedTransformContainer::get_depth_stream_intrinsics()
{
  return this->session->get_depth_stream_intrinsics();
}

std::shared_ptr<rs2_extrinsics> FixedTransformContainer::get_color_to_depth_extrinsics()
{
  return this->session->get_color_to_depth_extrinsics();
}

std::shared_ptr<rs2_extrinsics> FixedTransformContainer::get_depth_to_color_extrinsics()
{
  return this->session->get_depth_to_color_extrinsics();
}

float FixedTransformContainer::depth_at_pixel(int x, int y)
{
  float depth = this->session->depth_at_pixel(x,y);
  std::shared_ptr<cv::Point2f> point = std::make_shared<cv::Point2f>(x,y);
  std::shared_ptr<std::vector<std::shared_ptr<std::pair<std::shared_ptr<cv::Point2f>, float>>>> pixel_with_depth
      = std::make_shared<std::vector<std::shared_ptr<std::pair<std::shared_ptr<cv::Point2f>, float>>>>();
  pixel_with_depth->push_back(std::make_shared<std::pair<std::shared_ptr<cv::Point2f>, float>>(point, depth));
  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> cloud =
      this->session->get_point_cloud(this->transform, true, __FUNCTION__)->as_pixel_cloud()->new_point_cloud(pixel_with_depth);
  float new_depth = (*cloud->as_local_cloud()->get_data())(2,0);
  return new_depth;
}