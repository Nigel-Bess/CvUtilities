//
// Created by nkaffine on 11/21/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains code to ensure that translated and untranslated
 * point clouds work properly converting from one coordinate
 * system to the other.
 */
#include <gtest/gtest.h>
#include <Fulfil.DepthCam/point_cloud.h>
#include "../src/point_cloud/untranslated_point_cloud.h"
#include "../src/point_cloud/translated_point_cloud.h"
#include <eigen3/Eigen/Geometry>

using std::shared_ptr;
using std::make_shared;
using fulfil::depthcam::pointcloud::PointCloud;
using fulfil::depthcam::pointcloud::LocalPointCloud;
using fulfil::depthcam::pointcloud::CameraPointCloud;
using fulfil::depthcam::pointcloud::UntranslatedPointCloud;
using fulfil::depthcam::pointcloud::TranslatedPointCloud;

std::shared_ptr<Eigen::Matrix3Xd> generate_test_matrix()
{
  std::shared_ptr<Eigen::Matrix3Xd> test_mat = make_shared<Eigen::Matrix3Xd>(3,10);
  for(int i = 0; i < 10; i++)
  {
    (*test_mat)(0, i) = i;
    (*test_mat)(1, i) = 10 + i;
    (*test_mat)(2, i) = 20 + i;
  }
  return test_mat;
}

std::shared_ptr<Eigen::Affine3d> generate_test_transformation(int axis_to_flip)
{
  shared_ptr<Eigen::Affine3d> test_transform = make_shared<Eigen::Affine3d>();
  test_transform->linear() = Eigen::Matrix3d::Identity(3, 3);
  test_transform->translation() = Eigen::Vector3d::Zero();
  test_transform->linear()(axis_to_flip,axis_to_flip) = -1;
  return test_transform;
}

void assert_same_matrix(shared_ptr<Eigen::Matrix3Xd> matrix1, shared_ptr<Eigen::Matrix3Xd> matrix2)
{
  ASSERT_EQ(matrix1->size(), matrix2->size());
  ASSERT_EQ(matrix1->cols(), matrix2->cols());
  for(int i = 0; i < matrix1->cols(); i++)
  {
    ASSERT_EQ((*matrix1)(0,i), (*matrix2)(0,i));
    ASSERT_EQ((*matrix1)(1,i), (*matrix2)(1,i));
    ASSERT_EQ((*matrix1)(2,i), (*matrix2)(2,i));
  }
}

TEST(translationPointCloudTests, testUntranslatedInitialization)
{
  shared_ptr<Eigen::Matrix3Xd> matrix = generate_test_matrix();
  shared_ptr<Eigen::Affine3d> transform = generate_test_transformation(2);
  std::shared_ptr<PointCloud> point_cloud = make_shared<UntranslatedPointCloud>(matrix, transform, nullptr, nullptr, nullptr);
  std::shared_ptr<Eigen::Matrix3Xd> local_cloud_data = point_cloud->as_local_cloud()->get_data();
  std::shared_ptr<Eigen::Matrix3Xd> camera_cloud_data = point_cloud->as_camera_cloud()->get_data();
  for(int i = 0; i < 10; i++)
  {
    ASSERT_EQ((*local_cloud_data)(0,i), i);
    ASSERT_EQ((*local_cloud_data)(1,i), 10 + i);
    ASSERT_EQ((*local_cloud_data)(2, i), -1 * (20 + i));
    ASSERT_EQ((*camera_cloud_data)(0,i), i);
    ASSERT_EQ((*camera_cloud_data)(1,i), 10 + i);
    ASSERT_EQ((*camera_cloud_data)(2,i), 20 + i);
  }
}

TEST(translationPointCloudTests, testTranslatedInitialization)
{
  shared_ptr<Eigen::Matrix3Xd> matrix = generate_test_matrix();
  shared_ptr<Eigen::Affine3d> transform = generate_test_transformation(2);
  shared_ptr<Eigen::Matrix3Xd> transformed_matrix = make_shared<Eigen::Matrix3Xd>((*transform) * (*matrix));
  shared_ptr<PointCloud> point_cloud = make_shared<TranslatedPointCloud>(transformed_matrix, transform, nullptr, nullptr, nullptr);
  shared_ptr<Eigen::Matrix3Xd> local_cloud_data = point_cloud->as_local_cloud()->get_data();
  shared_ptr<Eigen::Matrix3Xd> camera_cloud_data = point_cloud->as_camera_cloud()->get_data();
  for(int i = 0; i < 10; i++)
  {
    ASSERT_EQ((*local_cloud_data)(0,i), i);
    ASSERT_EQ((*local_cloud_data)(1,i), 10 + i);
    ASSERT_EQ((*local_cloud_data)(2,i), -1 * (20 + i));
    ASSERT_EQ((*camera_cloud_data)(0,i), i);
    ASSERT_EQ((*camera_cloud_data)(1,i), 10 + i);
    ASSERT_EQ((*camera_cloud_data)(2,i), 20 + i);
  }
}

TEST(translationPointCloudTests, testTranslatedComparedToUntranslated)
{
  shared_ptr<Eigen::Matrix3Xd> matrix = generate_test_matrix();
  shared_ptr<Eigen::Affine3d> transformation = generate_test_transformation(2);
  shared_ptr<Eigen::Matrix3Xd> translated_matrix = make_shared<Eigen::Matrix3Xd>((*transformation) * (*matrix));
  shared_ptr<PointCloud> point_cloud_1 = make_shared<UntranslatedPointCloud>(matrix, transformation, nullptr, nullptr,nullptr);
  shared_ptr<PointCloud> point_cloud_2 = make_shared<TranslatedPointCloud>(translated_matrix, transformation, nullptr, nullptr,nullptr);
  //This illustrates the simplicity of this design.
  assert_same_matrix(point_cloud_1->as_local_cloud()->get_data(), point_cloud_2->as_local_cloud()->get_data());
  assert_same_matrix(point_cloud_1->as_camera_cloud()->get_data(), point_cloud_2->as_camera_cloud()->get_data());
}

TEST(translationPointCloudTests, testAddTransformationUntranslatedCloud)
{
  shared_ptr<Eigen::Matrix3Xd> matrix = generate_test_matrix();
  shared_ptr<Eigen::Affine3d> transformation1 = generate_test_transformation(2);
  std::shared_ptr<PointCloud> point_cloud = make_shared<UntranslatedPointCloud>(matrix, transformation1, nullptr, nullptr,nullptr);
  shared_ptr<Eigen::Affine3d> transformation2 = generate_test_transformation(1);
  point_cloud = point_cloud->add_transformation(transformation2);
  shared_ptr<Eigen::Matrix3Xd> local_point_data = point_cloud->as_local_cloud()->get_data();
  shared_ptr<Eigen::Matrix3Xd> camera_point_data = point_cloud->as_camera_cloud()->get_data();
  for(int i = 0; i < 10; i++)
  {
    ASSERT_EQ((*local_point_data)(0,i), i);
    ASSERT_EQ((*local_point_data)(1,i), -1 * (10 + i));
    ASSERT_EQ((*local_point_data)(2,i), -1 * (20 + i));
    ASSERT_EQ((*camera_point_data)(0,i), i);
    ASSERT_EQ((*camera_point_data)(1,i), 10 + i);
    ASSERT_EQ((*camera_point_data)(2,i), 20 + i);
  }
}

TEST(translationPointCloudTests, testAddTransformationTranslatedCloud)
{
  shared_ptr<Eigen::Matrix3Xd> matrix = generate_test_matrix();
  shared_ptr<Eigen::Affine3d> transformation1 = generate_test_transformation(2);
  shared_ptr<Eigen::Matrix3Xd> translated_matrix = make_shared<Eigen::Matrix3Xd>((*transformation1) * (*matrix));
  shared_ptr<PointCloud> point_cloud = make_shared<TranslatedPointCloud>(translated_matrix, transformation1,nullptr, nullptr,nullptr);
  shared_ptr<Eigen::Affine3d> transformation2 = generate_test_transformation(1);
  point_cloud = point_cloud->add_transformation(transformation2);
  shared_ptr<Eigen::Matrix3Xd> local_point_data = point_cloud->as_local_cloud()->get_data();
  shared_ptr<Eigen::Matrix3Xd> camera_point_data = point_cloud->as_camera_cloud()->get_data();
  for(int i = 0; i < 10; i++)
  {
    ASSERT_EQ((*local_point_data)(0,i), i);
    ASSERT_EQ((*local_point_data)(1,i), -1 * (10 + i));
    ASSERT_EQ((*local_point_data)(2,i), -1 * (20 + i));
    ASSERT_EQ((*camera_point_data)(0,i), i);
    ASSERT_EQ((*camera_point_data)(1,i), 10 + i);
    ASSERT_EQ((*camera_point_data)(2,i), 20 + i);
  }
}

TEST(translationPointCloudTests, testEquals)
{
  shared_ptr<Eigen::Matrix3Xd> matrix = generate_test_matrix();
  shared_ptr<Eigen::Affine3d> transformation1 = generate_test_transformation(2);
  shared_ptr<Eigen::Matrix3Xd> translated_matrix = make_shared<Eigen::Matrix3Xd>((*transformation1) * (*matrix));
  shared_ptr<PointCloud> point_cloud1 = make_shared<UntranslatedPointCloud>(matrix, transformation1, nullptr, nullptr,nullptr);
  shared_ptr<PointCloud> point_cloud2 = make_shared<TranslatedPointCloud>(translated_matrix, transformation1, nullptr, nullptr,nullptr);
  ASSERT_TRUE(point_cloud1->equal(point_cloud2));
  ASSERT_TRUE(point_cloud2->equal(point_cloud1));
  ASSERT_TRUE(point_cloud1->equal(point_cloud1));
  ASSERT_TRUE(point_cloud2->equal(point_cloud2));
}

TEST(translationPointCloudTests, testEqualsWithAddedTransformation)
{
  shared_ptr<Eigen::Matrix3Xd> matrix = generate_test_matrix();
  shared_ptr<Eigen::Affine3d> transformation1 = generate_test_transformation(2);
  shared_ptr<Eigen::Matrix3Xd> translated_matrix = make_shared<Eigen::Matrix3Xd>((*transformation1) * (*matrix));
  shared_ptr<PointCloud> point_cloud1 = make_shared<UntranslatedPointCloud>(matrix, transformation1, nullptr, nullptr,nullptr);
  shared_ptr<PointCloud> point_cloud2 = make_shared<TranslatedPointCloud>(translated_matrix, transformation1, nullptr, nullptr,nullptr);
  shared_ptr<Eigen::Affine3d> transformation2 = generate_test_transformation(1);
  point_cloud1 = point_cloud1->add_transformation(transformation2);
  point_cloud2 = point_cloud2->add_transformation(transformation2);
  ASSERT_TRUE(point_cloud1->equal(point_cloud1));
  ASSERT_TRUE(point_cloud1->equal(point_cloud2));
  ASSERT_TRUE(point_cloud2->equal(point_cloud2));
  ASSERT_TRUE(point_cloud2->equal(point_cloud1));
}

TEST(translationPointCloudTests, testNewPointCloudTransformedPointCloud)
{
  shared_ptr<Eigen::Matrix3Xd> matrix = generate_test_matrix();
  shared_ptr<Eigen::Affine3d> transformation1 = generate_test_transformation(2);
  shared_ptr<Eigen::Matrix3Xd> translated_matrix = make_shared<Eigen::Matrix3Xd>((*transformation1) * (*matrix));
  shared_ptr<PointCloud> point_cloud = make_shared<TranslatedPointCloud>(translated_matrix, transformation1, nullptr, nullptr,nullptr);
  std::shared_ptr<Eigen::Matrix3Xd> new_point_cloud_data = make_shared<Eigen::Matrix3Xd>(3,1);
  (*new_point_cloud_data) << 100,101,102;
  std::shared_ptr<PointCloud> new_local_cloud = point_cloud->as_local_cloud()->new_point_cloud(new_point_cloud_data);
  std::shared_ptr<PointCloud> new_camera_cloud = point_cloud->as_camera_cloud()->new_point_cloud(new_point_cloud_data);


  ASSERT_EQ((*new_local_cloud->as_local_cloud()->get_data()), *new_point_cloud_data);
  ASSERT_EQ((*new_camera_cloud->as_camera_cloud()->get_data()), *new_point_cloud_data);

  std::shared_ptr<Eigen::Matrix3Xd> inverted_new_point_cloud_data = make_shared<Eigen::Matrix3Xd>(3,1);
  (*inverted_new_point_cloud_data) << 100,101,-102;

  ASSERT_EQ((*new_local_cloud->as_camera_cloud()->get_data()), *inverted_new_point_cloud_data);
  ASSERT_EQ((*new_camera_cloud->as_local_cloud()->get_data()), *inverted_new_point_cloud_data);
}

TEST(translationPointCloudtests, testNewPointCloudUntransformedPointCloud)
{
  shared_ptr<Eigen::Matrix3Xd> matrix = generate_test_matrix();
  shared_ptr<Eigen::Affine3d> transformation1 = generate_test_transformation(2);
  shared_ptr<PointCloud> point_cloud = make_shared<TranslatedPointCloud>(matrix, transformation1,nullptr, nullptr,nullptr);
  std::shared_ptr<Eigen::Matrix3Xd> new_point_cloud_data = make_shared<Eigen::Matrix3Xd>(3,1);
  (*new_point_cloud_data) << 100,101,102;
  std::shared_ptr<PointCloud> new_local_cloud = point_cloud->as_local_cloud()->new_point_cloud(new_point_cloud_data);
  std::shared_ptr<PointCloud> new_camera_cloud = point_cloud->as_camera_cloud()->new_point_cloud(new_point_cloud_data);


  ASSERT_EQ((*new_local_cloud->as_local_cloud()->get_data()), *new_point_cloud_data);
  ASSERT_EQ((*new_camera_cloud->as_camera_cloud()->get_data()), *new_point_cloud_data);

  std::shared_ptr<Eigen::Matrix3Xd> inverted_new_point_cloud_data = make_shared<Eigen::Matrix3Xd>(3,1);
  (*inverted_new_point_cloud_data) << 100,101,-102;

  ASSERT_EQ((*new_local_cloud->as_camera_cloud()->get_data()), *inverted_new_point_cloud_data);
  ASSERT_EQ((*new_camera_cloud->as_local_cloud()->get_data()), *inverted_new_point_cloud_data);
}