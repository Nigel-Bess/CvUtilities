//
// Created by nkaffine on 12/2/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains the implemenation of encoding the
 * intrinsics of a sensor from the realsense library.
 */
#include "intrinsics_coder.h"
#include <cstring>
#include <fstream>
#include <ostream>

using fulfil::depthcam::pointcloud::IntrinsicsCoder;
using std::shared_ptr;
using std::make_shared;
using std::string;

char* IntrinsicsCoder::encode(shared_ptr<rs2_intrinsics> intrinsics)
{
    /**
     * From the realsense documentation:
     *  typedef struct rs_intrinsics
     *  {
     *      int           width;
     *      int           height;
     *      float         ppx;
     *      float         ppy;
     *      float         fx;
     *      float         fy;
     *      rs_distortion model;
     *      float         coeffs[5];
     * } rs_intrinsics;
     */
    size_t buffer_size = 2 * sizeof(int) + 9 * sizeof(float) + sizeof(rs2_distortion);
    char* buffer = new char[buffer_size];
    memcpy(&buffer[0], &(*intrinsics), buffer_size);
    return buffer;
}

std::shared_ptr<rs2_intrinsics> IntrinsicsCoder::decode(char *encoded_intrinsics)
{
    shared_ptr<rs2_intrinsics> intrinsics = make_shared<rs2_intrinsics>();
    int index = 0;
    memcpy(&intrinsics->width, &encoded_intrinsics[index], sizeof(int));
    index += sizeof(int);
    memcpy(&intrinsics->height, &encoded_intrinsics[index], sizeof(int));
    index += sizeof(int);
    memcpy(&intrinsics->ppx, &encoded_intrinsics[index], sizeof(float));
    index += sizeof(float);
    memcpy(&intrinsics->ppy, &encoded_intrinsics[index], sizeof(float));
    index += sizeof(float);
    memcpy(&intrinsics->fx, &encoded_intrinsics[index], sizeof(float));
    index += sizeof(float);
    memcpy(&intrinsics->fy, &encoded_intrinsics[index], sizeof(float));
    index += sizeof(float);
    memcpy(&intrinsics->model, &encoded_intrinsics[index], sizeof(rs2_distortion));
    index += sizeof(rs2_distortion);
    for(int i = 0; i < 5; i++)
    {
        memcpy(&intrinsics->coeffs[i], &encoded_intrinsics[index], sizeof(float));
        index += sizeof(float);
    }
    return intrinsics;
}

void IntrinsicsCoder::encode_to_file(shared_ptr<rs2_intrinsics> intrinsics, shared_ptr<std::string> filepath)
{
    char* encoded_intrinsics = encode(intrinsics);
    size_t write_size = 12 * sizeof(float);
    std::ofstream outfile(filepath->c_str());
    outfile.write(encoded_intrinsics, write_size);
    outfile.close();
    free(encoded_intrinsics);
}

std::shared_ptr<rs2_intrinsics> IntrinsicsCoder::decode_from_file(shared_ptr<std::string> filepath)
{
    std::ifstream infile(filepath->c_str());
    size_t read_size = 12 * sizeof(float);
    char* read_data = new char[read_size];
    infile.read(&read_data[0], read_size);
    shared_ptr<rs2_intrinsics> intrinsics = decode(read_data);
    free(read_data);
    infile.close();
    return intrinsics;
}