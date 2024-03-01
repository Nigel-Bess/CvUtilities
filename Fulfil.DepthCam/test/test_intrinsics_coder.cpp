//
// Created by nkaffine on 12/2/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains code that tests the implementation of
 * the intrinsics coder that encodes and decodes intrinsics
 * from the realsense library.
 */
#include "../src/coders/intrinsics_coder.h"
#include <memory>
#include <librealsense2/rs.h>
#include <gtest/gtest.h>

using std::shared_ptr;
using std::make_shared;
using std::string;
using fulfil::depthcam::pointcloud::IntrinsicsCoder;

std::shared_ptr<rs2_intrinsics> generate_sample_intrinsics()
{
    shared_ptr<rs2_intrinsics> intrinsics = make_shared<rs2_intrinsics>();
    intrinsics->width = 0;
    intrinsics->height = 1;
    intrinsics->ppx = 2;
    intrinsics->ppy = 3;
    intrinsics->fx = 4;
    intrinsics->fy = 5;
    intrinsics->model = RS2_DISTORTION_NONE;
    for(int i = 0; i < 5; i++)
    {
        intrinsics->coeffs[i] = 5+i;
    }
    return intrinsics;
}

void is_sample_intrinsics(shared_ptr<rs2_intrinsics> intrinsics)
{
    shared_ptr<rs2_intrinsics> sample_intrinsics = generate_sample_intrinsics();
    ASSERT_EQ(intrinsics->width, sample_intrinsics->width);
    ASSERT_EQ(intrinsics->height, sample_intrinsics->height);
    ASSERT_EQ(intrinsics->ppx, sample_intrinsics->ppx);
    ASSERT_EQ(intrinsics->ppy, sample_intrinsics->ppy);
    ASSERT_EQ(intrinsics->fx, sample_intrinsics->fx);
    ASSERT_EQ(intrinsics->fy, sample_intrinsics->fy);
    ASSERT_EQ(intrinsics->model, sample_intrinsics->model);
    for(int i = 0; i < 5; i++)
    {
        ASSERT_EQ(intrinsics->coeffs[i], sample_intrinsics->coeffs[i]);
    }
}

TEST(intrinsicCoderTests, testEncodingAndDecoding)
{
    shared_ptr<rs2_intrinsics> intrinsics = generate_sample_intrinsics();
    char* encoded_intrinsics = IntrinsicsCoder::encode(intrinsics);
    shared_ptr<rs2_intrinsics> decoded_intrinsics = IntrinsicsCoder::decode(encoded_intrinsics);
    is_sample_intrinsics(decoded_intrinsics);
}

TEST(intrinsicCoderTests, testEncodingDecodingFromFile)
{
    shared_ptr<rs2_intrinsics> intrinsics = generate_sample_intrinsics();
    shared_ptr<std::string> filename = make_shared<string>("intrinsics");
    IntrinsicsCoder::encode_to_file(intrinsics, filename);
    shared_ptr<rs2_intrinsics> decoded_intrinsics = IntrinsicsCoder::decode_from_file(filename);
    is_sample_intrinsics(decoded_intrinsics);
}

TEST(intrinsicCoderTests, testDecodingFromFile)
{
    shared_ptr<rs2_intrinsics> intrinsics = IntrinsicsCoder::decode_from_file(make_shared<string>("../../test/intrinsics"));
    is_sample_intrinsics(intrinsics);
}