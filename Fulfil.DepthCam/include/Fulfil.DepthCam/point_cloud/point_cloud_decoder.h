//
// Created by nkaffine on 11/20/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * The purpose of this file is to outline the functionality
 * of decoding point clouds that have been encoded to files.
 */
#ifndef FULFIL_DEPTHCAM_SRC_CODERS_POINT_CLOUD_DECODER_H_
#define FULFIL_DEPTHCAM_SRC_CODERS_POINT_CLOUD_DECODER_H_

#include <Fulfil.DepthCam/point_cloud/point_cloud.h>
#include <memory>

namespace fulfil
{
namespace depthcam
{
namespace pointcloud
{
class PointCloudDecoder
{
 private:
  static std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> decode_no_translation_point_cloud(
      std::shared_ptr<std::string> directory_path);
  static std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> decode_untranslated_point_cloud(
      std::shared_ptr<std::string> directory_path);
 public:
  /**
   * Uses the data in the given directory path to construct a point cloud and return it.
   * @param directory_path the path to the data where the point cloud has been encoded.
   * @return a pointer to the decoded point cloud.
   * @throws exception if the directory doesn't exist or the data is malformed.
   */
  static std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> decode_from_directory(
      std::shared_ptr<std::string> directory_path);
};
}
}
}
#endif //FULFIL_DEPTHCAM_SRC_CODERS_POINT_CLOUD_DECODER_H_
