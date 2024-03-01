//
// Created by nkaffine on 12/2/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains the implementation of encoding and
 * decoding extrinsics of a sensor from the realsense library.
 */
#include "extrinsics_coder.h"
#include <fstream>
#include <iostream>
#include <cstring>

using fulfil::depthcam::pointcloud::ExtrinsicsCoder;
using std::shared_ptr;
using std::make_shared;
using std::string;

char* ExtrinsicsCoder::encode(shared_ptr<rs2_extrinsics> extrinsics)
{
    /**
     * From the realsense documentation:
     * typedef struct rs_extrinsics
     * {
     *  float rotation[9];
     *  float translation[3];
     *  } rs_extrinsics;
     */
    char* buffer = new char[12 * sizeof(float)];
    memcpy(&buffer[0], &(*extrinsics), 12 * sizeof(float));
    return buffer;
}

std::shared_ptr<rs2_extrinsics> ExtrinsicsCoder::decode(char *encoded_extrinsics)
{
    shared_ptr<rs2_extrinsics> extrinsics = make_shared<rs2_extrinsics>();
    for(int i = 0; i < 9; i++)
    {
        memcpy(&extrinsics->rotation[i],&encoded_extrinsics[sizeof(float) * i], sizeof(float));
    }
    for(int i = 0; i < 3; i++)
    {
        memcpy(&extrinsics->translation[i], &encoded_extrinsics[(9+i)*sizeof(float)], sizeof(float));
    }
    return extrinsics;
}

std::shared_ptr<rs2_extrinsics> ExtrinsicsCoder::decode_from_file(shared_ptr<std::string> filepath)
{
    std::ifstream infile(filepath->c_str());
    size_t read_size = 12 * sizeof(float);
    char* read_data = new char[read_size];
    infile.read(&read_data[0], read_size);
    shared_ptr<rs2_extrinsics> extrinsics = decode(read_data);
    free(read_data);
    infile.close();
    return extrinsics;
}

void ExtrinsicsCoder::encode_to_file(shared_ptr<rs2_extrinsics> extrinsics, shared_ptr<std::string> filepath)
{
    char* encoded_extrinsics = encode(extrinsics);
    size_t write_size = 12 * sizeof(float);
    std::ofstream outfile(filepath->c_str());
    outfile.write(encoded_extrinsics, write_size);
    outfile.close();
    free(encoded_extrinsics);
}