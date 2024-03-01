//
// Created by nkaffine on 12/2/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file outlines the functionality for encoding
 * and decoding sensor extrinsics from the realsense
 * library.
 */
#ifndef FULFIL_DEPTHCAM_EXTRINSICS_CODER_H
#define FULFIL_DEPTHCAM_EXTRINSICS_CODER_H
#include <memory>
#include <librealsense2/rs.h>

namespace fulfil
{
namespace depthcam
{
namespace pointcloud
{
class ExtrinsicsCoder
{
 public:
  /**
   * Given realsense extrinsics, returns an encoding.
   * @param extrinsics pointer to the extrinsics to be encoded.
   * @return a pointer to the data of the extrinsics which are a fixed
   * length encoding.
   */
  static char* encode(std::shared_ptr<rs2_extrinsics> extrinsics);
  /**
   * Given realsense extrinsics and a filepath, writes the encoding of the
   * extrinsics to the filepath.
   * @param extrinsics pointer to the extrinsics to be encoded
   * @param filepath pointer to a string representing the filepath. If a file exists
   * it will write to the file, otherwise it will create a new file with the
   * encoding as its contents.
   */
  static void encode_to_file(std::shared_ptr<rs2_extrinsics> extrinsics, std::shared_ptr<std::string> filepath);
  /**
   * Given an encoding of realsense extrinsics, will return a decoded extrinsics pointer.
   * @param encoded_extrinsics a pointer to the data of the fixed length encoding of extrinsics
   * @return a pointer to the extrinsics object decoded from the encoding.
   */
  static std::shared_ptr<rs2_extrinsics> decode(char* encoded_extrinsics);
  /**
   * Given an filepath where an extrinsics encoding is stored, returns a decoded extrinsics
   * object.
   * @param filepath a pointer to a string representing the filepath.
   * @return a pointer to the extrinsics decoded from the file
   * @throws exception if file is not found.
   */
  static std::shared_ptr<rs2_extrinsics> decode_from_file(std::shared_ptr<std::string> filepath);
};
} // namespace fulfil
} // namespace core
} // namespace pointcloud

#endif
