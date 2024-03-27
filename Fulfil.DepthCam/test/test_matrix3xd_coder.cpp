//
// Created by nkaffine on 12/2/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains code that tests the implementation of the
 * Matrix3xdCoder that encodes and decodes matrix3xd objects
 * from the eigen library.
 */
#include "Fulfil.DepthCam/coders/matrix3xd_coder.h"
#include <eigen3/Eigen/Geometry>
#include <gtest/gtest.h>
#include <memory>

using fulfil::depthcam::pointcloud::Matrix3xdCoder;
using std::make_shared;
using std::shared_ptr;
using std::string;

//Testing to see if decoding an encoded matrix works.
TEST(pointCloudCoderTests, testEncodeDecode)
{
    shared_ptr<Eigen::Matrix3Xd> matrix = make_shared<Eigen::Matrix3Xd>(3, 10);
    *matrix << 0,3,6,9,12,15,18,21,24,27,
            1,4,7,10,13,16,19,22,25,28,
            2,5,8,11,14,17,20,23,26,29;
    char* encoded_mat = Matrix3xdCoder::encode(matrix);
    shared_ptr<Eigen::Matrix3Xd> decoded_matrix = Matrix3xdCoder::decode(encoded_mat);
    for(int i = 0; i < matrix->cols(); i++)
    {
        EXPECT_EQ((*matrix)(0,i), (*decoded_matrix)(0,i));
        EXPECT_EQ((*matrix)(1,i), (*decoded_matrix)(1,i));
        EXPECT_EQ((*matrix)(2,i), (*decoded_matrix)(2,i));
    }
}

TEST(pointCloudCoderTests, testEncodeDecodeToFile)
{
    shared_ptr<Eigen::Matrix3Xd> matrix = make_shared<Eigen::Matrix3Xd>(3, 20);
    *matrix << 0,3,6,9,12,15,18,21,24,27,30,33,36,39,42,45,48,51,54,57,
            1,4,7,10,13,16,19,22,25,28,31,34,37,40,43,46,49,52,55,58,
            2,5,8,11,14,17,20,23,26,29,32,35,38,41,44,47,50,53,56,59;
    shared_ptr<std::string> filename = make_shared<string>("test_point_cloud");
    Matrix3xdCoder::encode_to_file(matrix, filename);
    shared_ptr<Eigen::Matrix3Xd> read_matrix = Matrix3xdCoder::decode_from_file(filename);
    for(int i = 0; i < matrix->cols(); i++)
    {
        EXPECT_EQ((*matrix)(0,i), (*read_matrix)(0,i));
        EXPECT_EQ((*matrix)(1,i), (*read_matrix)(1,i));
        EXPECT_EQ((*matrix)(2,i), (*read_matrix)(2,i));
    }
}

TEST(pointCloudCoderTests, testDecodeBigFile)
{
    shared_ptr<std::string> filename = make_shared<string>("../../test/point_cloud");
    shared_ptr<Eigen::Matrix3Xd> read_matrix = Matrix3xdCoder::decode_from_file(filename);
    ASSERT_EQ(read_matrix->size(), 11571);
}