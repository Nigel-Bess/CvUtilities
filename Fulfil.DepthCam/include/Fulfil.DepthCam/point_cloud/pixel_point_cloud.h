//
// Created by nkaffine on 11/22/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * The purpose of this file is to outline the functionality
 * specific to a point cloud that is in the coordinate system
 * of pixels with depth values. It offers the ability to get
 * data as a vector of pixels as well as a vector of pairs
 * of pixels and depth values.
 */
#ifndef FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_POINT_CLOUD_PIXEL_POINT_CLOUD_H_
#define FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_POINT_CLOUD_PIXEL_POINT_CLOUD_H_

#include <vector>
#include <opencv2/opencv.hpp>
#include <memory>
#include <Fulfil.DepthCam/point_cloud/point_cloud.h>

namespace fulfil
{
namespace depthcam
{
namespace pointcloud
{
class PixelPointCloud : virtual public PointCloud
{
 public:
  /**
   * Returns a pointer to the pixels that are in the this point cloud
   * @return a pointer to the pixels in this point cloud.
   * @note this function returns a pointer to the data so any mutations
   * will have effects on the actual underlying data.
   */
  virtual std::shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>> get_data() = 0;
  /**
   * Returns a pointer to a vector of pairs of pixels and depth data that are
   * stored in this point cloud.
   * @return a pointer to a vector of pairs of pixels and the associated depth value.
   * @note this function returns a pointer to the data so any mutations
   * will have effects on the actual underlying data.
   */
  virtual std::shared_ptr<std::vector<std::shared_ptr<std::pair<std::shared_ptr<cv::Point2f>, float>>>>
    get_data_with_depth() = 0;

  [[nodiscard]] virtual cv::Size get_size_point_cloud_as_frame(float decimation_factor) const = 0;
  /**
   * Creates a new point cloud in the pixel coordinate system with the given pixels and depth values.
   * @param cloud a pointer to a vector of pairs of pixels and corresponding depth data that will be
   * used as the data for the resulting point cloud.
   * @return a pointer to the new point cloud.
   */
  virtual std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> new_point_cloud(
      std::shared_ptr<std::vector<std::shared_ptr<std::pair<std::shared_ptr<cv::Point2f>, float>>>> cloud) = 0;
};
}
}
}
#endif //FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_POINT_CLOUD_PIXEL_POINT_CLOUD_H_
