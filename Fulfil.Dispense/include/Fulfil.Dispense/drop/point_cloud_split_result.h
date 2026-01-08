#pragma once
#include <vector>
#include <memory>

using std::vector;
using std::shared_ptr;
using Eigen::Vector3d;

struct PointCloudSplitResult {
  shared_ptr<vector<int>> in_bag_point_indices;
  shared_ptr<vector<int>> out_of_bag_point_indices;
  inline Eigen::Matrix3Xd sub_sample(const Eigen::Matrix3Xd& all_points, bool insideBag){
    const auto indices = insideBag ? in_bag_point_indices : out_of_bag_point_indices;
    const auto m = static_cast<Eigen::Index>(indices->size());
    Eigen::Matrix3Xd out(3, m);
    for (Eigen::Index j = 0; j < m; ++j) out.col(j) = all_points.col((*indices)[(size_t)j]);
    return out;
  }
};