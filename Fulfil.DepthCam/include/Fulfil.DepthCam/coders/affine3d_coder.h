//
// Created by nkaffine on 11/20/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file outlines the functionality for encoding and decoding
 * an affine3d transformation from the eigen library.
 */
#ifndef FULFIL_DEPTHCAM_SRC_CODERS_AFFINE3D_CODER_H_
#define FULFIL_DEPTHCAM_SRC_CODERS_AFFINE3D_CODER_H_

#include <memory>
#include <eigen3/Eigen/Geometry>

namespace fulfil
{
namespace depthcam
{
namespace pointcloud
{
class Affine3DCoder
{
 public:
  /**
   * Given an transform, creates and encoding and returns the encoding paired with the size
   * in bytes of the encoding.
   * @param transform affine3d eigen transformation
   * @return a point to a pair containing a pointer to the encoded data and a long representing the
   * size of the data in bytes.
   */
  static std::shared_ptr<std::pair<char*, long>> encode(std::shared_ptr<Eigen::Affine3d> transform);
  /**
   * Given a transformation and a destination filepath, encodes the transform and writes the
   * encoding to the file at file path or creates a new file at the given filepath.
   * @param transform affine3d eigen transformation
   * @param filepath pointer to a string representing the filepath.
   */
  static void encode_to_file(std::shared_ptr<Eigen::Affine3d> transform, std::shared_ptr<std::string> filepath);
  /**
   * Given a pair of encoding and size, decodes the transformation it represents.
   * @param encoding a pointer to a pair containing the pointer to the data and a long representing the size
   * in bytes of the data.
   * @return affine3d eigen transformation decoded from the input.
   */
  static std::shared_ptr<Eigen::Affine3d> decode(std::shared_ptr<std::pair<char*, long>> encoding);
  /**
   * Given a filepath, return the decoded transformation from the data stored in that file.
   * @param filepath pointer to a string representing the filepath.
   * @return affine3d eigen transformation decoded from the file at the given filepath
   * @throws exception when the file does not exist.
   */
  static std::shared_ptr<Eigen::Affine3d> decode_from_file(std::shared_ptr<std::string> filepath);
};
}
}
}

#endif //FULFIL_DEPTHCAM_SRC_CODERS_AFFINE3D_CODER_H_
