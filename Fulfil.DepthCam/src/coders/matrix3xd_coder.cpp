//
// Created by nkaffine on 12/2/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains the implementation for encoding and decoding
 * a matrix3xd from the eigen library.
 */
#include "Fulfil.DepthCam/coders/matrix3xd_coder.h"
#include <fstream>
#include <iostream>

using fulfil::depthcam::pointcloud::Matrix3xdCoder;
using std::shared_ptr;
using std::make_shared;
using std::string;

char* Matrix3xdCoder::encode(shared_ptr<Eigen::Matrix3Xd> matrix)
{
    long matrix_byte_size = matrix->size() * sizeof(double);
    double* points = new double[matrix->size()];
    for(int i = 0; i < matrix->cols(); i++)
    {
        int x_index = i * 3;
        int y_index = x_index + 1;
        int z_index = y_index + 1;
        points[x_index] = (*matrix)(0,i);
        points[y_index] = (*matrix)(1,i);
        points[z_index] = (*matrix)(2,i);
    }
    char* buffer = new char[matrix_byte_size + sizeof(long)];
    long matrix_size = matrix->size();
    memcpy(&buffer[0], &matrix_size, sizeof(long));
    for(int i = 0; i < matrix->size(); i++)
    {
        int char_index = (i * sizeof(double)) + sizeof(long);
        memcpy(&buffer[char_index], &points[i], sizeof(double));
    }
    free(points);
    return buffer;
}

std::shared_ptr<Eigen::Matrix3Xd> Matrix3xdCoder::decode(char *encoded_matrix)
{
    long size;
    memcpy(&size, encoded_matrix, sizeof(long));
    double* mat_data = (double*) &encoded_matrix[sizeof(long)];
    //checking that the size of the matrix is valid.
    if((size % 3) != 0)
    {
        throw std::invalid_argument("invalid encoded matrix");
    }
    assert((size % 3) == 0);
    shared_ptr<Eigen::Matrix3Xd> matrix = make_shared<Eigen::Matrix3Xd>(3, size/3);
    for(int i = 0; i < size / 3; i++)
    {
        int x_index = i * 3;
        int y_index = x_index + 1;
        int z_index = y_index + 1;
        memcpy(&(*matrix)(0,i), &mat_data[x_index], sizeof(double));
        memcpy(&(*matrix)(1,i), &mat_data[y_index], sizeof(double));
        memcpy(&(*matrix)(2,i), &mat_data[z_index], sizeof(double));
    }
    return matrix;
}

void Matrix3xdCoder::encode_to_file(shared_ptr<Eigen::Matrix3Xd> matrix, shared_ptr<std::string> filename)
{
    char* encoded_matrix = Matrix3xdCoder::encode(matrix);
    long write_size = matrix->size() * sizeof(double) + sizeof(long);
    std::ofstream outfile(filename->c_str());
    outfile.write(encoded_matrix, write_size);
    outfile.close();
    free(encoded_matrix);
}

std::shared_ptr<Eigen::Matrix3Xd> Matrix3xdCoder::decode_from_file(shared_ptr<std::string> filename)
{
    std::ifstream infile(filename->c_str());
    long mat_size;
    infile.read((char*) &mat_size, sizeof(long));
    long read_size = mat_size * sizeof(double);
    char* read_data = new char[read_size + sizeof(long)];
    memcpy(&read_data[0], &mat_size, sizeof(long));
    infile.read(&read_data[sizeof(long)], read_size);
    shared_ptr<Eigen::Matrix3Xd> final_mat = Matrix3xdCoder::decode(read_data);
    free(read_data);
    infile.close();
    return final_mat;
}