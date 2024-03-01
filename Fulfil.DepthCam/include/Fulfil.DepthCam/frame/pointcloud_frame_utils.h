//
// Created by amber on 11/10/22.
//

#ifndef FULFIL_DISPENSE_POINTCLOUD_FRAME_UTILS_H
#define FULFIL_DISPENSE_POINTCLOUD_FRAME_UTILS_H
#include <opencv2/opencv.hpp>
#include <array>
#include <Fulfil.DepthCam/point_cloud.h>


namespace fulfil::depthcam::pointcloud_frame_utils {



// double decimated_frame_dimension(int frame_dimension, int decimation_factor=8);

// std::array<double, 2> decimated_frame_size(int original_frame_width=1280,
//    int original_frame_height=720, int decimation_factor=8);

// std::array<int, 2> decimated_frame_index(int x, int y, int decimation_factor=8); todo

//std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud>& point_cloud_to_depth_frame(
//    std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud);

[[no_discard]] std::tuple<int, std::array<float, 3>>  get_max_height(const Eigen::Matrix3Xd& depth_data);




}

#endif// FULFIL_DISPENSE_POINTCLOUD_FRAME_UTILS_H
