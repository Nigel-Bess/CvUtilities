//
// Created by amber on 10/16/20.
//

#include "Fulfil.Dispense/tray/tray.h"
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.Dispense/tray/measurement_helpers.h>


using fulfil::dispense::tray::Tray;
using fulfil::utils::Logger;

// TODO consolidate the tray width and fiducial width params into shift
Tray::Tray(const nlohmann::json &tray_recipe, float tray_width, float fiducial_width_offset)
{
  double origin_x_shift = fulfil::measure::tray_origin_xoffset_from_edge(tray_width, fiducial_width_offset);
  this->tray_type_name = tray_recipe["Name"].get<std::string>();
  this->lane_count = tray_recipe["Lane_Count"].get<int>();
  this->max_item_width = fulfil::measure::to_meters(tray_recipe["Max_Item_Width"].get<float>());
  this->min_item_width = fulfil::measure::to_meters(tray_recipe["Min_Item_Width"].get<float>());
  auto get_centers = [&origin_x_shift](const std::vector<double>& lane_iterable) {
    if (fulfil::measure::recipe_centers_left_of_reference_edge(lane_iterable)) {
      return fulfil::measure::disp_gen3_recipe_centers_to_tray_frame(lane_iterable, origin_x_shift);
    }
    return fulfil::measure::disp_gen2_recipe_centers_to_tray_frame(lane_iterable, origin_x_shift);
  };
  this->lane_centers = get_centers(tray_recipe["Lane_Centers"].get<std::vector<double>>());
  this->valid_tray = (min_item_width != 0 && max_item_width != 0 &&
          !this->lane_centers.empty() && lane_count == this->lane_centers.size());
}


Tray::Tray(dimensional_info::TrayRecipe &tray_recipe, float tray_width, float fiducial_width_offset)
{
    double origin_x_shift = fulfil::measure::tray_origin_xoffset_from_edge(tray_width, fiducial_width_offset);
    this->valid_tray = tray_recipe.is_valid();
    if (this->valid_tray) {
        this->tray_type_name = tray_recipe.m_name;
        this->lane_count = tray_recipe.m_lane_count;
        this->max_item_width = fulfil::measure::to_meters(tray_recipe.m_max_item_width);
        this->min_item_width = fulfil::measure::to_meters(tray_recipe.m_min_item_width);
        auto get_centers = [&origin_x_shift, &lane_iterable=tray_recipe.m_lane_center_locs]() {
            if (fulfil::measure::recipe_centers_left_of_reference_edge(lane_iterable)) {
                return fulfil::measure::disp_gen3_recipe_centers_to_tray_frame(lane_iterable, origin_x_shift);
            }
            return fulfil::measure::disp_gen2_recipe_centers_to_tray_frame(lane_iterable, origin_x_shift);
        };
        this->lane_centers=get_centers();
    }

}

double Tray::get_lane_center(int lane_index) const
{
  if (lane_index >= this->lane_count || lane_index < 0)
  { return -1; } // TODO Bad error action

  return this->lane_centers.at(lane_index);
}

int Tray::get_lane_count() const
{
  return lane_count;
}

float Tray::get_max_item_width() const
{
  return max_item_width;
}

float Tray::get_min_item_width() const
{
  return min_item_width;
}

bool Tray::is_valid() const
{
  return valid_tray;
}

std::string_view Tray::get_tray_name() const
{
  return tray_type_name;
}

// TODO, Need to pull line creation code out and just pass this data in, which will
// remove the repetition here and mean that we don't have to create a gutter for each lane
Eigen::Matrix3Xd Tray::get_lane_gutters(const Eigen::Matrix3Xd& lane_center_coordinates, double z_gutter_shift) {
  Eigen::Matrix3Xd lane_gutters (3, lane_center_coordinates.cols()*2);
  double lane_width_offset = min_item_width/2;
  //A(all, Eigen::ArithmeticSequence::seq(0,last,fix<2>))

  Eigen::Vector3d dimension_shift ({-lane_width_offset,  0, z_gutter_shift});
  lane_gutters << lane_center_coordinates.colwise() + dimension_shift;

  dimension_shift.x() *= -1;
  lane_gutters << lane_center_coordinates.colwise() + dimension_shift;
  return lane_gutters;

}
// fulfil::measure::make_eigen_row_from_vector(std::vector<double> row_vec)
/*
 Eigen::Matrix3Xd Tray::get_tray_center_coordinates(double tray_length, double z_plane)
{
  using RowInitMapXd = typename Eigen::Map<Eigen::RowVectorXd, Eigen::Aligned>;
    Eigen::Matrix3Xd lane_center_coordinates {3, lane_count*2};
    lane_center_coordinates << RowInitMapXd(lane_centers.data(), lane_count),
            RowInitMapXd(lane_centers.data(), lane_count),
            Eigen::RowVectorXd::Constant( lane_count,  (tray_length/ 2)),
            Eigen::RowVectorXd::Constant( lane_count, -(tray_length/ 2 )),
            Eigen::RowVectorXd::Constant( static_cast<long>(lane_count)*2, (z_plane));
    return lane_center_coordinates;
}

 Eigen::Matrix3Xd Tray::get_lane_line_coordinates(double tray_length, std::vector<double> item_search_heights) {
   using RowInitMapXd = typename Eigen::Map<Eigen::RowVectorXd, Eigen::Aligned>;
   Eigen::Matrix3Xd lane_center_coordinates (3, lane_count*2);
   lane_center_coordinates << RowInitMapXd(lane_centers.data(), lane_count),
       RowInitMapXd(lane_centers.data(), lane_count),
       Eigen::RowVectorXd::Constant( lane_count, (tray_length/ 2)),
       Eigen::RowVectorXd::Constant( lane_count, -(tray_length/ 2)),
       RowInitMapXd(item_search_heights.data(), lane_count),
       RowInitMapXd(item_search_heights.data(), lane_count);
   return lane_center_coordinates;
}
 */

Eigen::Matrix3Xd Tray::get_tray_center_coordinates(double tray_length, double z_plane)
{
  double origin_to_y_edge = (tray_length/ 2);
  Eigen::Matrix3Xd lane_center_coordinates {3, lane_count*2};
  lane_center_coordinates <<
      fulfil::measure::make_eigen_row_from_vector(lane_centers),
      fulfil::measure::make_eigen_row_from_vector(lane_centers),
      fulfil::measure::make_eigen_row_from_constant(origin_to_y_edge, lane_count),
      fulfil::measure::make_eigen_row_from_constant(-1*origin_to_y_edge, lane_count),
      fulfil::measure::make_eigen_row_from_constant(z_plane, static_cast<long>(lane_count)*2);
  return lane_center_coordinates;
}


Eigen::Matrix3Xd Tray::get_lane_line_coordinates(double tray_length, const std::vector<double>& item_search_heights) {
  double origin_to_y_edge = (tray_length/ 2);
  Eigen::Matrix3Xd lane_center_coordinates {3, lane_count*2};
  lane_center_coordinates <<
      fulfil::measure::make_eigen_row_from_vector(lane_centers),
      fulfil::measure::make_eigen_row_from_vector(lane_centers),
      fulfil::measure::make_eigen_row_from_constant(origin_to_y_edge, lane_count),
      fulfil::measure::make_eigen_row_from_constant(-1*origin_to_y_edge, lane_count),
      fulfil::measure::make_eigen_row_from_vector(item_search_heights),
      fulfil::measure::make_eigen_row_from_vector(item_search_heights);
  return lane_center_coordinates;
}


