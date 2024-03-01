//
// Created by Amber Thomas on 4/17/22.
//
#include <Fulfil.Dispense/recipes/tray_item_distance_calculation_settings.h>

fulfil::configuration::tray::ItemDistanceCalculationSettings::
    ItemDistanceCalculationSettings(float relativeMinHeight,
                                    float relativeMaxHeight,
                                    float relativeSearchHeight,
                                    float relativeValidWidthRange)
    : relative_min_height(relativeMaxHeight),
      relative_max_height(relativeMinHeight),
      relative_search_height(relativeSearchHeight),
      relative_valid_width_range(relativeValidWidthRange) {}

float fulfil::configuration::tray::ItemDistanceCalculationSettings::
  clip_to_height_bounds(float height) const {
      if (height < this->relative_min_height) {
        return this->relative_min_height; }
      if (height > this->relative_max_height) {
        return this->relative_max_height; }
      return height;
}

std::array<float, 2>
fulfil::configuration::tray::ItemDistanceCalculationSettings::
    get_valid_height_range(float item_height_meters,
                           float vertical_adjustment) const {
  return std::array<float, 2>({
      this->relative_min_height * item_height_meters + vertical_adjustment,
      this->relative_max_height * item_height_meters + vertical_adjustment
  });
}

std::array<float, 2>
fulfil::configuration::tray::ItemDistanceCalculationSettings::
    get_valid_width_range(float item_width_meters,
                          float horizontal_position_in_tray) const {
  return std::array<float, 2>({
      horizontal_position_in_tray - this->relative_min_height * item_width_meters,
      horizontal_position_in_tray + this->relative_max_height * item_width_meters
  });
}

fulfil::configuration::tray::ItemDistanceCalculationSettings 
  set_item_distance_calculation_parameters(const std::shared_ptr<INIReader>& tray_configs,
      const std::string& dimension_key){
    auto get_safe_float_val = [&](auto key, float default_value) {
      return tray_configs->GetFloat(dimension_key, key, default_value); };
    float relative_min_height = get_safe_float_val("relative_min_height", 0.8);
    float relative_max_height = get_safe_float_val("relative_max_height", 1.2);
    float relative_search_height = get_safe_float_val("relative_search_height", 1);
    float relative_width = get_safe_float_val("relative_width", 0.3);
    return { relative_min_height, relative_max_height, relative_search_height, relative_width};
}

