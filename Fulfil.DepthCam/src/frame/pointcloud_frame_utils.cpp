//
// Created by amber on 11/10/22.
//
#include <Fulfil.DepthCam/frame/pointcloud_frame_utils.h>
#include <Fulfil.CPPUtils/logging.h>

using fulfil::depthcam::pointcloud::PointCloud;
using fulfil::depthcam::pointcloud::LocalPointCloud;
/*
std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud>& point_cloud_to_depth_frame(
    std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud, double max_depth, double min_depth) {
  auto depth_pixels = point_cloud->as_pixel_cloud()->get_data_with_depth();

  std::shared_ptr<LocalPointCloud> local_point_cloud = point_cloud->as_pixel_cloud()->new_point_cloud(depth_pixels)->as_local_cloud();
  auto local_depth_data = local_point_cloud->get_data();

  // Display in a GUI
  float max_dist = -1000;
  float min_dist = 1000;
  for(int i = 0; i < depth_pixels->size(); i++)
  {
    //std::cout << depth_pixels->at(i)->second;
    max_dist = std::max(max_dist, (float) (*local_depth_data)(2,i));
    min_dist = std::min(min_dist, (float) (*local_depth_data)(2,i));

    def calc(full_img_dim, dec_factor):
t = full_img_dim//dec_factor + 3
t //= 4
t *= 4
return t
}
*/




std::tuple<int, std::array<float, 3>> fulfil::depthcam::pointcloud_frame_utils::get_max_height(const Eigen::Matrix3Xd& depth_data){
  Eigen::Index max_height_index;
  double max_depth = depth_data.row(2).minCoeff(&max_height_index);
  std::array<float, 3>  max_pt = {
    static_cast<float>(depth_data.col(max_height_index).x()),
    static_cast<float>(depth_data.col(max_height_index).y()),
    static_cast<float>(depth_data.col(max_height_index).z()) };
  fulfil::utils::Logger::Instance()->Info("The max height is located at {} in pc, with a value of {}, full point is ({},{},{})",
      max_height_index, max_depth, max_pt[0], max_pt[1], max_pt[2]);
  return std::tuple(max_height_index, max_pt);
}

