//
// Created by nkaffine on 12/2/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file includes code that tests the implementation of the
 * extrinsics coder that encodes and decodes extrinsics from
 * the realsense library.
 */
#include "../src/coders/extrinsics_coder.h"
#include <gtest/gtest.h>
#include <memory>

using fulfil::depthcam::pointcloud::ExtrinsicsCoder;
using std::shared_ptr;
using std::make_shared;
using std::string;

TEST(extrinsicCoderTests, testEncodingAndDecoding)
{
   std::shared_ptr<rs2_extrinsics> extrinsics = make_shared<rs2_extrinsics>();
    for(int i = 0; i < 9; i++)
    {
        extrinsics->rotation[i] = i;
    }
    for(int i = 0; i < 3; i++)
    {
        extrinsics->translation[i] = i+9;
    }
    char* encoding = ExtrinsicsCoder::encode(extrinsics);
   std::shared_ptr<rs2_extrinsics> decoded_extrinsics = ExtrinsicsCoder::decode(encoding);
    for(int i = 0; i < 9; i++)
    {
        ASSERT_EQ(extrinsics->rotation[i], decoded_extrinsics->rotation[i]);
    }
    for(int i = 0; i < 3; i++)
    {
        ASSERT_EQ(extrinsics->translation[i], decoded_extrinsics->translation[i]);
    }
}

TEST(extrinsicCoderTests, testEncodingDecodingFromFile)
{
   std::shared_ptr<rs2_extrinsics> extrinsics = make_shared<rs2_extrinsics>();
    for(int i = 0; i < 9; i++)
    {
        extrinsics->rotation[i] = i;
    }
    for(int i = 0; i < 3; i++)
    {
        extrinsics->translation[i] = i+9;
    }
    shared_ptr<std::string> filepath = make_shared<string>("test_coding");
    ExtrinsicsCoder::encode_to_file(extrinsics, filepath);
   std::shared_ptr<rs2_extrinsics> decoded_extrinsics = ExtrinsicsCoder::decode_from_file(filepath);
    for(int i = 0; i < 9; i++)
    {
        ASSERT_EQ(extrinsics->rotation[i], decoded_extrinsics->rotation[i]);
    }
    for(int i = 0; i < 3; i++)
    {
        ASSERT_EQ(extrinsics->translation[i], decoded_extrinsics->translation[i]);
    }
}

TEST(extrinsicCoderTests, testDecodingFromFile)
{
    shared_ptr<std::string> filename = make_shared<string>("../../test/extrinsics");
   std::shared_ptr<rs2_extrinsics> extrinsics = ExtrinsicsCoder::decode_from_file(filename);
    for(int i = 0; i < 9; i++)
    {
        ASSERT_EQ(extrinsics->rotation[i], i);
    }
    for(int i = 0; i < 3; i++)
    {
        ASSERT_EQ(extrinsics->translation[i], i+9);
    }
}