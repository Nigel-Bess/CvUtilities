//
// Created by Amber Thomas on 4/16/22.
//

#ifndef FULFIL_DISPENSE_TRAY_DIMENSIONAL_CONFIGURATION_H
#define FULFIL_DISPENSE_TRAY_DIMENSIONAL_CONFIGURATION_H
#include <string>
#include "Fulfil.Dispense/tray/tray.h"
#include <Fulfil.CPPUtils/inih/INIReader.h>

namespace fulfil::configuration::tray {
    class TrayDimensions {
        private:
            float tray_length{};
            float tray_width{};
            float fiducial_width_offset{}; // 0.03363 is default for 2.1
            float dispense_arm_height{0.039};
            float lane_inset_from_edge{0.024};
            // Bumper offset?

        public:
          TrayDimensions()=default;

          TrayDimensions(float trayLength, float trayWidth,
                             float fiducialWidthOffset, float dispenseArmHeight,
                             float laneInsetFromEdge);
          TrayDimensions(float trayLength, float trayWidth,
                         float fiducialWidthOffset);

          // Generic to allow use of parsed or json obj
          template <typename Recipe>
          fulfil::dispense::tray::Tray build_tray_from_recipe(Recipe tray_data){
              return fulfil::dispense::tray::Tray(tray_data, this->tray_width, this->fiducial_width_offset);
          }

          [[nodiscard]] float get_length() const;
          [[nodiscard]] float get_width() const;
          [[nodiscard]] float get_fiducial_width_offset() const;
          [[nodiscard]] float get_dispense_arm_popthru_height() const;
          [[nodiscard]] float get_lane_inset_from_edge() const;
    };

    fulfil::configuration::tray::TrayDimensions set_bay_wide_tray_dimensions(
        const std::shared_ptr<INIReader> &tray_config_reader,
        const std::string &tray_generation_key);
}

#endif // FULFIL_DISPENSE_TRAY_DIMENSIONAL_CONFIGURATION_H
