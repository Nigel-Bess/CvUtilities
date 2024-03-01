//
// Created by nkaffine on 11/21/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains code that test teh implementation of the
 * no translation point cloud to ensure that the conversions from
 * one coordinate system to another work correctly.
 */
#include <gtest/gtest.h>
#include "../src/point_cloud/no_translation_point_cloud.h"
#include <eigen3/Eigen/Geometry>
#include <Fulfil.DepthCam/point_cloud.h>

using std::shared_ptr;
using std::make_shared;
using fulfil::depthcam::pointcloud::PointCloud;
using fulfil::depthcam::pointcloud::NoTranslationPointCloud;
using fulfil::depthcam::pointcloud::LocalPointCloud;
using fulfil::depthcam::pointcloud::CameraPointCloud;

TEST(testNoTranslationPointCloud, testLocalAndCameraCloudsAreSame)
{
  shared_ptr<Eigen::Matrix3Xd> cloud_data = make_shared<Eigen::Matrix3Xd>(3,10);
  for(int i = 0; i < 10; i++)
  {
    (*cloud_data)(0,i) = i;
    (*cloud_data)(1,i) = 10 + i;
    (*cloud_data)(2,i) = 20 + i;
  }
  shared_ptr<PointCloud> point_cloud = make_shared<NoTranslationPointCloud>(cloud_data, nullptr, nullptr, nullptr);
  std::shared_ptr<Eigen::Matrix3Xd> local_cloud_data = point_cloud->as_local_cloud()->get_data();
  std::shared_ptr<Eigen::Matrix3Xd> camera_cloud_data = point_cloud->as_camera_cloud()->get_data();
  for(int i = 0; i < 10; i++)
  {
    ASSERT_EQ((*local_cloud_data)(0,i), i);
    ASSERT_EQ((*local_cloud_data)(1,i), 10 + i);
    ASSERT_EQ((*local_cloud_data)(2,i), 20 + i);
    ASSERT_EQ((*camera_cloud_data)(0,i), i);
    ASSERT_EQ((*camera_cloud_data)(1,i), 10 + i);
    ASSERT_EQ((*camera_cloud_data)(2,i), 20 + i);
  }
}

TEST(testNoTranslationPointCloud, testAddTranslation)
{
  std::shared_ptr<Eigen::Affine3d> transformation = make_shared<Eigen::Affine3d>();
  transformation->linear() = Eigen::Matrix3Xd::Identity(3,3);
  transformation->translation() = Eigen::Vector3d::Zero();
  transformation->linear()(2,2) = -1;

  shared_ptr<Eigen::Matrix3Xd> cloud_data = make_shared<Eigen::Matrix3Xd>(3,10);
  for(int i = 0; i < 10; i++)
  {
    (*cloud_data)(0,i) = i;
    (*cloud_data)(1,i) = 10 + i;
    (*cloud_data)(2,i) = 20 + i;
  }
  shared_ptr<PointCloud> point_cloud = make_shared<NoTranslationPointCloud>(cloud_data, nullptr, nullptr, nullptr);
  point_cloud = point_cloud->add_transformation(transformation);
  shared_ptr<Eigen::Matrix3Xd> new_camera_cloud_data = point_cloud->as_camera_cloud()->get_data();
  shared_ptr<Eigen::Matrix3Xd> new_local_cloud_data = point_cloud->as_local_cloud()->get_data();
  for(int i = 0; i < 10; i++)
  {
    ASSERT_EQ((*new_camera_cloud_data)(0,i), i);
    ASSERT_EQ((*new_camera_cloud_data)(1,i), 10 + i);
    ASSERT_EQ((*new_camera_cloud_data)(2,i), 20 + i);
    ASSERT_EQ((*new_local_cloud_data)(0,i), i);
    ASSERT_EQ((*new_local_cloud_data)(1,i), 10 + i);
    ASSERT_EQ((*new_local_cloud_data)(2,i), -1 * (20 + i));
  }
}

TEST(testNoTranslationPointCloud, testEquals)
{
  std::shared_ptr<Eigen::Affine3d> transformation = make_shared<Eigen::Affine3d>();
  transformation->linear() = Eigen::Matrix3Xd::Identity(3,3);
  transformation->translation() = Eigen::Vector3d::Zero();
  transformation->linear()(2,2) = -1;

  shared_ptr<Eigen::Matrix3Xd> cloud_data = make_shared<Eigen::Matrix3Xd>(3,10);
  for(int i = 0; i < 10; i++)
  {
    (*cloud_data)(0,i) = i;
    (*cloud_data)(1,i) = 10 + i;
    (*cloud_data)(2,i) = 20 + i;
  }
  shared_ptr<PointCloud> point_cloud = make_shared<NoTranslationPointCloud>(cloud_data, nullptr, nullptr, nullptr);
  ASSERT_TRUE(point_cloud->equal(make_shared<NoTranslationPointCloud>(cloud_data, nullptr, nullptr, nullptr)));
  shared_ptr<PointCloud> translated_point_cloud = point_cloud->add_transformation(transformation);
  ASSERT_FALSE(point_cloud->equal(translated_point_cloud));

  std::shared_ptr<Eigen::Affine3d> identity_transform = make_shared<Eigen::Affine3d>();
  identity_transform->linear() = Eigen::Matrix3Xd::Identity(3,3);
  identity_transform->translation() = Eigen::Vector3d::Zero();
  shared_ptr<PointCloud> identity_translated_point_cloud = point_cloud->add_transformation(identity_transform);
  ASSERT_TRUE(point_cloud->equal(identity_translated_point_cloud));
}

TEST(testNoTranslationPointCloud, testNewPointCloud)
{
  shared_ptr<Eigen::Matrix3Xd> cloud_data = make_shared<Eigen::Matrix3Xd>(3,10);
  for(int i = 0; i < 10; i++)
  {
    (*cloud_data)(0,i) = i;
    (*cloud_data)(1,i) = 10 + i;
    (*cloud_data)(2,i) = 20 + i;
  }
  shared_ptr<PointCloud> point_cloud = make_shared<NoTranslationPointCloud>(cloud_data, nullptr, nullptr, nullptr);
  shared_ptr<Eigen::Matrix3Xd> new_cloud_data = make_shared<Eigen::Matrix3Xd>(3,1);
  *new_cloud_data << 100,101,102;
  shared_ptr<PointCloud> new_local_cloud = point_cloud->as_local_cloud()->new_point_cloud(new_cloud_data);
  shared_ptr<PointCloud> new_camera_cloud = point_cloud->as_camera_cloud()->new_point_cloud(new_cloud_data);
  ASSERT_TRUE(new_local_cloud->equal(new_camera_cloud));
  ASSERT_FALSE(new_local_cloud->equal(point_cloud));
  ASSERT_FALSE(new_camera_cloud->equal(point_cloud));
  std::shared_ptr<Eigen::Matrix3Xd> mat = new_local_cloud->as_local_cloud()->get_data();
  ASSERT_EQ((*mat), (*new_cloud_data));
}