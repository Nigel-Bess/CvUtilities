//
// Created by nkaffine on 12/10/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_SRC_POINT_GRID_H_
#define FULFIL_DISPENSE_SRC_POINT_GRID_H_
#include <memory>
#include "Fulfil.CPPUtils/point_3d.h"
#include <vector>
#include "Fulfil.CPPUtils/pixel.h"
#include <opencv2/opencv.hpp>

namespace fulfil
{
namespace dispense {
namespace drop
{
/**
 * The purpose of this class is to provide a discrete
 * grid that a point cloud can be adapted to to enable
 * working in discrete coordinates instead of continuous
 * ones.
 */
class DropGrid
{
 private:

  /**
   * The number of rows in grid
   */
  int num_rows;
  /**
   * The number of cols in the grid
   */
  int num_cols;
  /**
   *  Width of grid in mm (vertical side of LFB in images, in line with grid rows and X local coordinates)
   */
  float grid_width;
  /**
   *  Length of grid in mm (horizontal side of LFB in images, in line with grid columns and Y local coordinates)
   */
  float grid_length;

  float cell_length;
  float cell_width;

  float minx; // along width, in line with grid rows
  float miny; // along length, in line with grid columns

  float maxx;
  float maxy;

  void add_neighboring_cells(std::shared_ptr<cv::Mat> target_grid, int row, int col);

 public:
  /**
   * DropGrid constructor. Grid_width and grid_length are input with units of meters
   */
  DropGrid(float grid_width, float grid_length, int num_rows, int num_cols);

  /**
  * The grid of values (for now, just average depth values. Can add more channels later?)
  */
  cv::Mat depth_average_grid;
  cv::Mat depth_max_grid;
  cv::Mat depth_min_grid;
  cv::Mat depth_count_grid; //contains number of depth points assigned to each grid cell

  /**
   *  Takes in a Matrix3XD representation of a local point cloud's data and uses it to populate the depth grids
   *  TODO: call this function in the DropGrid constructor, require point cloud input to constructor
   */
  void populate_depth(std::shared_ptr<Eigen::Matrix3Xd> local_point_cloud_data);

  /**
   *  Evaluates whether an item with certain shadow dimensions (dimensions when rotated 90 degrees after dispense)
   *  can fit inside the bag represented by the DropGrid. Item fits if there is any region of the DropGrid covered by
   *  the item's shadow that has an average depth below upper_depth_limit
   *  Inputs: are all provided in meter units
   */
  bool check_whether_item_fits(float shadow_width, float shadow_length, float upper_depth_limit);

  /**
   * Create a probability grid based on the known dispense target
   * @param target_x  (LFB coordinate system, in meters)
   * @param target_y  (LFB coordinate system, in meters)
   * @param shadow_length (LFB coordinate system, in meters)
   * @param shadow_width (LFB coordinate system, in meters)
   * @return  cv::Mat, 100% probability corresponds to cell having value = 1, 0% = 0
   */
  cv::Mat get_grid_from_target(float target_x, float target_y, float shadow_length, float shadow_width);

};
} // namespace drop
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_SRC_POINT_GRID_H_
