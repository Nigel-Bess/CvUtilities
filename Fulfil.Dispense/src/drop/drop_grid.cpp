//
// Created by steveburkeff on 10/16/2020
// Copyright (c) 2022 Fulfil Solutions, Inc. All rights reserved.
//
#include <cmath>
#include "Fulfil.Dispense/drop/drop_grid.h"
#include <Fulfil.CPPUtils/logging.h>
#include <iostream>
#include <algorithm>

using GridRow = std::shared_ptr<std::vector<std::shared_ptr<fulfil::utils::Point3D>>>;
using fulfil::dispense::drop::DropGrid;
using fulfil::utils::Logger;


DropGrid::DropGrid(float grid_width, float grid_length, int num_rows, int num_cols)
{
  this->grid_width = grid_width;
  this->grid_length = grid_length;
  this->num_rows = num_rows;
  this->num_cols = num_cols;
  this->cell_length = grid_length / num_cols;
  this->cell_width = grid_width / num_rows;

  this->minx = -grid_width / 2.0;
  this->miny = -grid_length / 2.0;
  this->maxx = grid_width / 2.0;
  this->maxy = grid_length / 2.0;

  this->depth_average_grid = cv::Mat::zeros(num_rows, num_cols, CV_32F);
  this->depth_max_grid = cv::Mat(num_rows, num_cols, CV_32F,-1);
  this->depth_min_grid = cv::Mat(num_rows, num_cols, CV_32F,1);
  this->depth_count_grid = cv::Mat::zeros(num_rows, num_cols, CV_32F);
}

void DropGrid::populate_depth(std::shared_ptr<Eigen::Matrix3Xd> local_point_cloud_data)
{
  Eigen::Matrix3Xd data = *local_point_cloud_data;
  Logger::Instance()->Trace("Number of point cloud points to populate into drop grid: {}", data.cols());
  Logger::Instance()->Trace("Drop grid Minx: {}, Maxx: {}", this->minx, this->maxx);
  Logger::Instance()->Trace("Drop grid Miny: {}, Maxy: {}", this->miny, this->maxy);

  for(int i = 0; i < data.cols(); i++)
  {
    float x = data(0, i);
    float y = data(1, i);
    float z = data(2, i);

    Logger::Instance()->Trace("X: {}, Y: {}, Z: {}", x, y, z);

    //to prevent points being thrown out or over-indexing if they are on the very edge of the container
    if(x == maxx) x -= 0.001;
    if(y == maxy) y -= 0.001;
    if(x == minx) x += 0.001;
    if(y == miny) y += 0.001;

    //any point-cloud points outside of the container bounds (for example, if container was extended in post-drop image,
    //for detecting items sticking out over markers) are not included in the drop grid construction
    if(x <= this->minx or x >= this->maxx or y <= this->miny or y >= this->maxy or z < -0.5 or z > 1.0) continue;

    int row = (int)floor((x - this->minx)/(this->grid_width/this->num_rows));
    int col = (int)floor((y - this->miny)/(this->grid_length/this->num_cols));

    if(!(0 <= row and row <= (num_rows-1)))
    {
      Logger::Instance()->Error("Depth Grid Construction; Vars: x = {}, row = {}, minx = {}, grid_width = {}, num_rows = {}; Cam: LFB",
                                x, row, this->minx, this->grid_width, this->num_rows);
      continue;
    }
    if(!(0 <= col and col <= (num_cols-1)))
    {
      Logger::Instance()->Error("Depth Grid Construction; Vars: y = {}, col = {}, miny = {}, grid_length = {}, num_cols = {}; Cam: LFB",
                                y, col, this->miny, this->grid_length, this->num_cols);
      continue;
    }

    float current_average = this->depth_average_grid.at<float>(row,col);
    int current_square_count = this->depth_count_grid.at<float>(row,col);

    int new_count = current_square_count + 1;
    float new_average = ((current_average * current_square_count) + z) / new_count;

    //update min and max grids if appropriate
    if(z > this->depth_max_grid.at<float>(row,col)) this->depth_max_grid.at<float>(row,col) = z;
    if(z < this->depth_min_grid.at<float>(row,col)) this->depth_min_grid.at<float>(row,col) = z;

    this->depth_average_grid.at<float>(row, col) = new_average;
    this->depth_count_grid.at<float>(row,col) = new_count;
  }
}

bool DropGrid::check_whether_item_fits(float shadow_width, float shadow_length, float upper_depth_limit)
{
    int num_rows_shadow =  std::ceil(shadow_width / this->cell_width);
    int num_cols_shadow =  std::ceil(shadow_length / this->cell_length);

    // So it looks like the num rows and cols is a maximum
    if(num_rows_shadow > this->num_rows || num_cols_shadow > this->num_cols)
    {
        Logger::Instance()->Error("DropGrid check_fit bound error; Vars: shadow_rows = {}, shadow_cols = {}, grid_rows = {}, grid_cols= {}; Cam: LFB",
                                  num_rows_shadow, num_cols_shadow, this->num_rows, this->num_cols);
        return false;
    }

    Logger::Instance()->Info("DropGrid: checking fit of item with dimensions: s-width ({}), s-length ({}), upper_depth_limit ({}), shadow_rows ({}), shadow_cols ({})",
                              shadow_width, shadow_length, upper_depth_limit, num_rows_shadow, num_cols_shadow);

    //the reference is the top left corner of the shadow (as viewed by the design diagram)
    int shadow_reference_max_row = this->num_rows - num_rows_shadow;
    int shadow_reference_max_col = this->num_cols - num_cols_shadow;

    Logger::Instance()->Debug("DropGrid: search shadow center has: max_row ({}), max_col ({})",
                              shadow_reference_max_row, shadow_reference_max_col);

    //cycle through each valid reference position for the shadow region
    for(int ref_row = 0; ref_row < shadow_reference_max_row + 1; ref_row ++){
        for(int ref_col = 0; ref_col < shadow_reference_max_col + 1; ref_col++){
            int num_valid_squares_shadow_region = 0;
            float depth_sum_shadow_region = 0;
            //find the average of valid depth squares in the region
            for(int row = ref_row; row < (ref_row + num_rows_shadow); row++){
                for (int col = ref_col; col < (ref_col + num_cols_shadow); col++){
                    if(!(0 <= row and row <= (this->num_rows-1) and 0 <= col and col <= (this->num_cols -1))) //error checking TODO: consider removing for speed
                    {
                        Logger::Instance()->Error("DropGrid: checking fit row/col error: ({}, {}, {}, {}); Cam: LFB",
                                                  row, col, num_rows, num_cols);
                        continue;
                    }
                    if(this->depth_count_grid.at<float>(row,col) > 0){
                        num_valid_squares_shadow_region += 1;
                        depth_sum_shadow_region += this->depth_average_grid.at<float>(row,col);
                    }
                }
            }
            if(num_valid_squares_shadow_region > 0)
            {
                float average_depth_shadow_region = depth_sum_shadow_region / num_valid_squares_shadow_region;
                if(average_depth_shadow_region < upper_depth_limit){
                    Logger::Instance()->Info("DropGrid: found valid place for item! Row: {}, Col: {}, valid squares: {} / {}, avg depth: {}",
                                             ref_row, ref_col, num_valid_squares_shadow_region, num_rows_shadow * num_cols_shadow, average_depth_shadow_region);
                    return true;
                }
            }
        }
    }
    Logger::Instance()->Info("DropGrid: no valid shadow regions found for item");
    return false;
}

void DropGrid::add_neighboring_cells(std::shared_ptr<cv::Mat> target_grid, int row, int col)
{
  // populate grid cells in the same column as the selected point
  float val = 0.2;
  for(int i = 0; i < this->num_rows; i++)
  {
    if (i != row)
    {
      if(target_grid->at<float>(i, col) == 0) target_grid->at<float>(i, col) = val;
    }
  }

  // populate grid cells in the same row
  val = 0.5;
  for(int i = 0; i < this->num_cols; i++)
  {
    if (i != col)
    {
      if(target_grid->at<float>(row, i) == 0) target_grid->at<float>(row, i) = val;
    }
  }

  // populate grid cells within one square (corners). Check for cases when neighbors would be out of bounds
  val = 0.8;
  if(col != 0)
  {
    if(target_grid->at<float>(row, col - 1) != 1) target_grid->at<float>(row, col - 1) = val;
  }
  if(col != (this->num_cols - 1))
  {
    if(target_grid->at<float>(row, col + 1) != 1) target_grid->at<float>(row, col + 1) = val;
  }
  if(row != 0)
  {
    if(target_grid->at<float>(row - 1, col) != 1) target_grid->at<float>(row - 1, col) = val;
  }
  if(row != (this->num_rows - 1))
  {
    if(target_grid->at<float>(row + 1, col) != 1) target_grid->at<float>(row + 1, col) = val;
  }
  if(row != 0 and col != 0)
  {
    if(target_grid->at<float>(row - 1, col - 1) != 1) target_grid->at<float>(row - 1, col - 1) = val;
  }
  if(row != 0 and col != (this->num_cols - 1))
  {
    if(target_grid->at<float>(row - 1, col + 1) != 1) target_grid->at<float>(row - 1, col + 1) = val;
  }
  if(row != (this->num_rows - 1) and col != 0)
  {
    if(target_grid->at<float>(row + 1, col - 1) != 1) target_grid->at<float>(row + 1, col - 1) = val;
  }
  if(row != (this->num_rows - 1) and col != (this->num_cols - 1))
  {
    if(target_grid->at<float>(row + 1, col + 1) != 1) target_grid->at<float>(row + 1, col + 1) = val;
  }
}

cv::Mat DropGrid::get_grid_from_target(float target_x, float target_y, float shadow_length, float shadow_width)
{
  //for this function, make sure shadow dim
  if (shadow_length < this->cell_length * 1.05) shadow_length = this->cell_length * 1.05;
  if (shadow_width < this->cell_width * 1.05) shadow_width = this->cell_width * 1.05;

  float target_min_x = target_x - shadow_width / 2.0;
  float target_max_x = target_x + shadow_width / 2.0;
  float target_min_y = target_y - shadow_length / 2.0;
  float target_max_y = target_y + shadow_length / 2.0;

  // Why is this in 32f?
  std::shared_ptr<cv::Mat> target_grid = std::make_shared<cv::Mat>(cv::Mat::zeros(this->num_rows, this->num_cols, CV_32F));

  // for any grid cells that have centers within the defined target region, set their grid values = 255
  for(int i = 0; i < this->num_rows; i++)
  {
    for(int j = 0; j < this->num_cols; j++)
    {
      float cell_center_x = this->minx + (this->cell_width) * i + (this->cell_width)/2.0;
      float cell_center_y = this->miny + (this->cell_length) * j + (this->cell_length)/2.0;

      if(cell_center_x > target_min_x and cell_center_x < target_max_x and cell_center_y > target_min_y and cell_center_y < target_max_y)
      {

        target_grid->at<float>(i, j) = 1;
        //add_neighboring_cells(target_grid, i, j);
      }
    }
  }
  return *target_grid;
}