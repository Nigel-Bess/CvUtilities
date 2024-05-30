//
// Created by nkaffine on 11/20/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains code that tests the functionality of the
 * coder that encodes and decods affine3d transformations
 * from the eigen library.
 */
#include <gtest/gtest.h>
#include "Fulfil.DepthCam/coders/affine3d_coder.h"
#include <eigen3/Eigen/Geometry>

using fulfil::depthcam::pointcloud::Affine3DCoder;

TEST(affine3dCoderTests, testEncodingDecoding)
{
  std::shared_ptr<Eigen::Affine3d> transformation = std::make_shared<Eigen::Affine3d>();
  for(int x = 0; x < 3; x++)
  {
    for(int y = 0; y < 3; y++)
    {
      transformation->linear()(x,y) = 3*y + x;
    }
    transformation->translation()(x) = 10 * x;
  }
  std::shared_ptr<std::pair<char*, long>> encoding = Affine3DCoder::encode(transformation);
  std::shared_ptr<Eigen::Affine3d> read_transformation = Affine3DCoder::decode(encoding);
  for(int x = 0; x < 3; x++)
  {
    for(int y = 0; y < 3; y++)
    {
      ASSERT_EQ(transformation->linear()(x,y), read_transformation->linear()(x,y));
    }
    ASSERT_EQ(transformation->translation()(x), read_transformation->translation()(x));
  }
}

TEST(affine3dCoderTests, testEncodingDecodingFromFile)
{
  std::shared_ptr<Eigen::Affine3d> transformation = std::make_shared<Eigen::Affine3d>();
  for(int x = 0; x < 3; x++)
  {
    for(int y = 0; y < 3; y++)
    {
      transformation->linear()(x,y) = 3*y + x;
    }
    transformation->translation()(x) = 10 * x;
  }
  Affine3DCoder::encode_to_file(transformation, std::make_shared<std::string>("tmp"));
  std::shared_ptr<Eigen::Affine3d> read_transformation = Affine3DCoder::decode_from_file(
      std::make_shared<std::string>("tmp"));
  system("rm tmp");

  for(int x = 0; x < 3; x++)
  {
    for(int y = 0; y < 3; y++)
    {
      ASSERT_EQ(transformation->linear()(x,y), read_transformation->linear()(x,y));
    }
    ASSERT_EQ(transformation->translation()(x), read_transformation->translation()(x));
  }
}
