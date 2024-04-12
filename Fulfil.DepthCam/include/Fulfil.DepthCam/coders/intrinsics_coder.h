//
// Created by nkaffine on 12/2/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file outlines the functionality of encoding and
 * decoding the intrinsics of a sensor from the realsense
 * library.
 */
#ifndef FULFIL_DEPTHCAM_INTRINSICS_CODER_H
#define FULFIL_DEPTHCAM_INTRINSICS_CODER_H
#include <memory>
#include <librealsense2/rs.h>

namespace fulfil
{
namespace depthcam
{
namespace pointcloud
{
class IntrinsicsCoder
{
 public:
  /**
   * Given realsense intrinsics, returns a fixed length encoding of the intrinsics
   * @param intrinsics a pointer to the intrinsics to be encoded
   * @return a pointer to the fixed length encoding of the intrinsics.
   */
  static char* encode(std::shared_ptr<rs2_intrinsics> intrinsics);
  /**
   * Given realsense intrinsics and a filepath, write the encoding of the intrinsics to the filepath.
   * @param intrinsics a pointer to the intrinsics to be encoded.
   * @param filepath a pointer to a string representing the filepath (if no file exists, one will be created).
   */
  static void encode_to_file(std::shared_ptr<rs2_intrinsics> intrinsics, std::shared_ptr<std::string> filepath);
  /**
   * Given an encoding of intrinsics, will return the decoded intrinsics object.
   * @param encoded_intrinsics pointer to the data from the fixed length encoding of the intrinsics.
   * @return a pointer to the intrinsics decoded from the encoding.
   */
  static std::shared_ptr<rs2_intrinsics> decode(char* encoded_intrinsics);
  /**
   * Given a filepath with an encoding of intrinsics, returns intrinsics decoded from the file data.
   * @param filepath a pointer to a string representing the filepath.
   * @return a pointer to the intrinsics object decoded from the contents of the file.
   */
  static std::shared_ptr<rs2_intrinsics> decode_from_file(std::shared_ptr<std::string> filepath);
};
} // namespace fulfil
} // namespace core
} // namespace pointcloud
#endif