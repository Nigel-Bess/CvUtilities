//
// Created by amber on 10/16/20.
//

#ifndef FULFIL_DISPENSE_TRAY_H
#define FULFIL_DISPENSE_TRAY_H
#include <memory>
#include <vector>
#include <eigen3/Eigen/Core>

#include <Fulfil.Dispense/json.hpp>
#include <Fulfil.Dispense/tray/tray_parser.h>


namespace fulfil
{
    namespace dispense {
        namespace tray
        {
/**
 * The purpose of this class is to define parameters of a tray using cached tray types
 */
            class Tray
            {
            private:
                std::string tray_type_name;
                float max_item_width {};
                float min_item_width {};
                int lane_count {-1};
                bool valid_tray {};
                std::vector<double> lane_centers;




            public:


                Tray(const nlohmann::json &tray_recipe, float tray_width, float fiducial_width_offset);

                Tray(dimensional_info::TrayRecipe &tray_recipe, float tray_width, float fiducial_width_offset);



                // TODO make invalid tray class
                Tray()=default; // invalid tray

                [[nodiscard]] double get_lane_center(int lane_index) const;

                [[nodiscard]] float get_max_item_width() const;

                [[nodiscard]] float get_min_item_width() const;

                [[nodiscard]] int get_lane_count() const;

                [[nodiscard]] bool is_valid() const;

                [[nodiscard]] std::string_view get_tray_name() const;

                //std::shared_ptr<Eigen::Matrix3Xd>
                //get_lane_line_coordinates(double tray_length, double z_plane=0, double back_extend=0, double front_extend=0);
                Eigen::Matrix3Xd get_lane_gutters(const Eigen::Matrix3Xd& lane_center_coordinates, double z_gutter_shift);


                Eigen::Matrix3Xd get_tray_center_coordinates(double tray_length, double z_plane=0);


                Eigen::Matrix3Xd get_lane_line_coordinates(double tray_length, const std::vector<double>& item_search_heights);
            };
        } // namespace tray
    } // namespace dispense
} // namespace fulfil


#endif //FULFIL_DISPENSE_TRAY_H
