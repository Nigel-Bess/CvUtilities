//
// Created by amber on 4/12/22.
//

#ifndef FULFIL_DISPENSE_MEASUREMENT_HELPERS_H
#define FULFIL_DISPENSE_MEASUREMENT_HELPERS_H
#include <vector>
#include <algorithm>
#include <eigen3/Eigen/Core>



namespace fulfil::measure {

    template<typename CoordinateIterable>
    bool recipe_centers_left_of_reference_edge(CoordinateIterable lane_locations)
    {
        return std::all_of(std::begin(lane_locations), std::end(lane_locations), [](auto i){return i < 0; });
    }

    float to_rounded_millimeters(float meters);
    float scale_and_round(float value, float scale);
    float to_meters(float millimeters);
    double tray_origin_xoffset_from_edge(float tray_width, float fiducial_width_offset) ;

    template<class CentersIter, class AccessScalingOp>
    std::vector<double> recipe_lane_centers_to_tray_frame(CentersIter first_center_ref, CentersIter last_center_ref,
                                                          AccessScalingOp to_tray_reference_frame) {
        std::vector<double> centers_in_tray_ref_frame;
        centers_in_tray_ref_frame.reserve(10);
        std::transform(first_center_ref, last_center_ref, std::back_inserter(centers_in_tray_ref_frame),
                       to_tray_reference_frame);
        return centers_in_tray_ref_frame;
    }

    std::vector<double> disp_gen2_recipe_centers_to_tray_frame(const std::vector<double> &tray_recipe_centers,
                                                               double origin_x_shift);

    std::vector<double> disp_gen3_recipe_centers_to_tray_frame(const std::vector<double> &tray_recipe_centers,
                                                               double origin_x_shift);

    Eigen::MatrixXd make_eigen_row_from_vector(std::vector<double> row_vec);

    Eigen::MatrixXd make_eigen_row_from_constant(double row_const, long num_cols);

    template<typename T>
    std::vector<T> fold_group_op(T first_in, T second_in) {
        return {first_in, second_in};
    }

    template<typename T>
    std::vector<T> fold_group_op(std::vector<T> first_in, std::vector<T> second_in) {
        return std::move(second_in.begin(), second_in.end(), std::back_inserter(first_in));
    }

    template<typename FirstInputIt, typename SecondInputIt, typename OutputIt>
    OutputIt folding_group_merge(FirstInputIt first_in, SecondInputIt second_in, OutputIt output_it)
    {
        return std::transform(first_in.begin(), first_in.end(), second_in.begin(), output_it,
                [&](auto first, auto second) { return fold_group_op(first, second); });
    }


}

#endif //FULFIL_DISPENSE_MEASUREMENT_HELPERS_H
