//
// Created by nkaffine on 11/20/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains the implementation of encoding
 * and decoding the affine3d transformation from the
 * eigen library.
 */
#include "affine3d_coder.h"
#include <Fulfil.CPPUtils/file_system_util.h>

using fulfil::depthcam::pointcloud::Affine3DCoder;
using fulfil::utils::FileSystemUtil;

std::shared_ptr<std::pair<char*, long>> Affine3DCoder::encode(std::shared_ptr<Eigen::Affine3d> transform)
{
  double* elements = (double*) malloc(sizeof(double) * 12);
  /**
   * The encoding will have (0,0) be at the index 0, and (0,1) be at index 3. The first 9 indices
   * will be for the rotation, the last three will be x,y,z for the translation respectively.
   */
  for(int x = 0; x < 3; x++)
  {
    for(int y = 0; y < 3; y++)
    {
      elements[y*3 + x] = transform->linear()(x,y);
    }
    elements[9 + x] = transform->translation()(x);
  }
  return std::make_shared<std::pair<char*, long>>((char*) elements, sizeof(double) * 12);
}

void Affine3DCoder::encode_to_file(std::shared_ptr<Eigen::Affine3d> transform, std::shared_ptr<std::string> filepath)
{
  std::shared_ptr<std::pair<char*, long>> data = encode(transform);
  FileSystemUtil::write_to_file(data->first, data->second, filepath->c_str());
  free(data->first);
}

std::shared_ptr<Eigen::Affine3d> Affine3DCoder::decode(std::shared_ptr<std::pair<char *, long>> encoding)
{
  double* elements = (double*) encoding->first;
  std::shared_ptr<Eigen::Affine3d> transformation = std::make_shared<Eigen::Affine3d>();
  for(int x = 0; x < 3; x++)
  {
    for(int y = 0; y < 3; y++)
    {
      transformation->linear()(x,y) = elements[y*3 + x];
    }
    transformation->translation()(x) = elements[9 + x];
  }
  return transformation;
}

std::shared_ptr<Eigen::Affine3d> Affine3DCoder::decode_from_file(std::shared_ptr<std::string> filepath)
{
  std::shared_ptr<std::string> content = FileSystemUtil::get_string_from_file(filepath->c_str());
  char* data_copy = (char*) malloc(12 * sizeof(double));
  std::memcpy(data_copy, content->c_str(), 12 * sizeof(double));
  std::shared_ptr<std::pair<char*, long>> data = std::make_shared<std::pair<char*, long>>(data_copy, 12 * sizeof(double));
  std::shared_ptr<Eigen::Affine3d> transformation = decode(data);
  free(data_copy);
  return transformation;
}