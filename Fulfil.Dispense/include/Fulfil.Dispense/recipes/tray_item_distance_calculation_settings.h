//
// Created by Amber Thomas on 4/17/22.
//

#ifndef FULFIL_DISPENSE_TRAY_ITEM_DISTANCE_CALCULATION_SETTINGS_H
#define FULFIL_DISPENSE_TRAY_ITEM_DISTANCE_CALCULATION_SETTINGS_H

#include <array>
#include <memory>
#include <Fulfil.CPPUtils/inih/INIReader.h>
namespace fulfil::configuration::tray {
    class ItemDistanceCalculationSettings {
    public:
      ItemDistanceCalculationSettings()=default;
      ItemDistanceCalculationSettings(float relativeMinHeight,
                                      float relativeMaxHeight,
                                      float relativeSearchHeight,
                                      float relativeValidWidthRange);

      [[nodiscard]] std::array<float, 2>
      get_valid_height_range(float item_height_meters,
                             float vertical_adjustment=0) const;

      [[nodiscard]] std::array<float, 2>
      get_valid_width_range(float item_width_meters,
                            float horizontal_position_in_tray=0) const;

    private:
              float relative_min_height{0.9};
              float relative_max_height{1.1};
              float relative_search_height{1};
              float relative_valid_width_range{0.3};

              [[nodiscard]] float clip_to_height_bounds(float height) const;
        };

  ItemDistanceCalculationSettings
        set_item_distance_calculation_parameters(const std::shared_ptr<INIReader>& tray_configs);



}
#endif // FULFIL_DISPENSE_TRAY_ITEM_DISTANCE_CALCULATION_SETTINGS_H
