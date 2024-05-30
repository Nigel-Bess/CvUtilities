//
// Created by amber on 4/12/22.
//

#include <Fulfil.Dispense/tray/measurement_helpers.h>
#include <cmath>

/*
float fulfil::measure::scale_and_round(float value, float scale)
{
    return std::round(value * scale);
    float fulfil::measure::to_rounded_millimeters(float meters)
{
    constexpr float scale = 1000;
    return fulfil::measure::scale_and_round(meters, scale);
}

double fulfil::measure::to_rounded_millimeters(double meters)
{
    constexpr float scale = 1000;
    return fulfil::measure::scale_and_round(meters, scale);
}

}*/


float fulfil::measure::to_meters(float millimeters)
{
    constexpr float scale = 1000;
    return millimeters/scale;
}

double fulfil::measure::tray_origin_xoffset_from_edge(float tray_width, float fiducial_width_offset) {
    return (tray_width/2) + fiducial_width_offset;
}

std::vector<double>
fulfil::measure::disp_gen2_recipe_centers_to_tray_frame(const std::vector<double> &tray_recipe_centers,
                                                        const double origin_x_shift) {
    auto scale_fn = [origin_x_shift] (double recipe_center) { return fulfil::measure::to_meters(recipe_center) - origin_x_shift; };
    return recipe_lane_centers_to_tray_frame(tray_recipe_centers.cbegin(), tray_recipe_centers.cend(), scale_fn);
}

std::vector<double>
fulfil::measure::disp_gen3_recipe_centers_to_tray_frame(const std::vector<double> &tray_recipe_centers,
                                                        const double origin_x_shift) {
    auto scale_fn = [origin_x_shift] (double recipe_center) { return fulfil::measure::to_meters(-1*recipe_center) - origin_x_shift; };
    return recipe_lane_centers_to_tray_frame(tray_recipe_centers.crbegin(), tray_recipe_centers.crend(), scale_fn);
}

Eigen::MatrixXd fulfil::measure::make_eigen_row_from_vector(std::vector<double> row_vec) {
  using RowInitMapXd = typename Eigen::Map<Eigen::RowVectorXd, Eigen::Aligned>;
  Eigen::MatrixXd row (1, row_vec.size());
  row << RowInitMapXd(row_vec.data(), static_cast<long>(row_vec.size()));
  return row;
}

Eigen::MatrixXd fulfil::measure::make_eigen_row_from_constant(double row_const, long num_cols) {
  Eigen::MatrixXd row (1, num_cols);
  row << Eigen::RowVectorXd::Constant(num_cols, row_const);
  return row;
}