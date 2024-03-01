//
// Created by nkaffine on 11/20/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file outlines the functionality for encoding and decoding
 * matrix3xd from the eigen library.
 */
#ifndef FULFIL_DEPTHCAM_POINT_CLOUD_CODER_H
#define FULFIL_DEPTHCAM_POINT_CLOUD_CODER_H
#include <eigen3/Eigen/Geometry>
#include <memory>

namespace fulfil
{
namespace depthcam
{
namespace pointcloud
{
class Matrix3xdCoder
{
 public:
  /**
   * Returns an encoding of the matrix.
   * @param matrix pointer to a 3xn eigen matrix
   * @return a dynamic length encoding of the matrix (size of the matrix
   * is encoded in the encoding)
   * @note the return value needs to be freed to prevent a memory leak.
   */
  static char* encode(std::shared_ptr<Eigen::Matrix3Xd> matrix);
  /**
   * Encodes the given matrix to the given filepath.
   * @param matrix pointer to a 3xn eigen matrix.
   * @param filename a pointer to a string representing the filepath to
   * write the encoding too (if no file exists, one will be created).
   */
  static void encode_to_file(std::shared_ptr<Eigen::Matrix3Xd> matrix, std::shared_ptr<std::string> filename);
  /**
   * Given an encoding of a matrix, returns the decoded matrix object.
   * @param encoded_matrix a pointer to the data of the encoding.
   * @return the decoded matrix object from the encoding.
   */
  static std::shared_ptr<Eigen::Matrix3Xd> decode(char* encoded_matrix);
  /**
   * Given a file with an encoded matrix, returns the decoded matrix object.
   * @param filename a pointer to a string that represents the filepath with the matrix data.
   * @return a pointer to the matrix object of the encoded matrix.
   * @note if the file does not contain a valid matrix or the file doesn't exist,
   * it will throw an exception.
   */
  static std::shared_ptr<Eigen::Matrix3Xd> decode_from_file(std::shared_ptr<std::string> filename);
};
} // namespace pointcloud
} // namespace core
} // namespace fulfil
#endif
