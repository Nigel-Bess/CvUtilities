//
// Created by Amber Thomas on 4/16/22.
//

#include <Fulfil.Dispense/recipes/tray_dimensional_configuration.h>
#include <Fulfil.CPPUtils/logging.h>


using namespace fulfil::configuration::tray;
// TODO think of better name
void valid_dimension_check(float trayLength, float trayWidth) {
  if (trayLength <= 0 || trayWidth <= 0) {
    fulfil::utils::Logger::Instance()->Fatal(
        "Tried to run with invalid dimensions width={}, height={}. "
        "Both values should be greater than 0. Fix it fool.",
        trayLength, trayWidth);
    exit(EXIT_FAILURE);
  }
}

TrayDimensions::TrayDimensions(float trayLength, float trayWidth,
                     float fiducialWidthOffset, float dispenseArmHeight,
                     float laneInsetFromEdge)
    : tray_length(trayLength), tray_width(trayWidth),
      fiducial_width_offset(fiducialWidthOffset),
      dispense_arm_height(dispenseArmHeight),
      lane_inset_from_edge(laneInsetFromEdge) {
  valid_dimension_check(trayLength, trayWidth);
}


TrayDimensions::TrayDimensions(float trayLength, float trayWidth,
                               float fiducialWidthOffset)
    : tray_length(trayLength), tray_width(trayWidth),
      fiducial_width_offset(fiducialWidthOffset) {
  valid_dimension_check(trayLength, trayWidth);
}

float TrayDimensions::get_length() const { return tray_length; }

float TrayDimensions::get_width() const { return tray_width; }

float TrayDimensions::get_fiducial_width_offset() const {
  return fiducial_width_offset;
}

float TrayDimensions::get_dispense_arm_popthru_height() const {
  return dispense_arm_height;
}

float TrayDimensions::get_lane_inset_from_edge() const {
  return lane_inset_from_edge;
}

TrayDimensions fulfil::configuration::tray::set_bay_wide_tray_dimensions(const std::shared_ptr<INIReader>& tray_configs,
                                            const std::string& tray_dimensions) {

  fulfil::utils::Logger::Instance()->Info("Reading tray config dimensions from section:{}", tray_dimensions);
  auto get_safe_float_val = [&](auto key, float default_value) { return tray_configs->GetFloat(tray_dimensions, key, default_value); };
  float tray_length = get_safe_float_val("tray_length", 0);
  float tray_width = get_safe_float_val("tray_width", 0);
  float fiducial_width_offset = get_safe_float_val("fiducial_width_offset",0);
  float dispense_arm_height = get_safe_float_val("dispense_arm_height", 0.039);
  float lane_inset = get_safe_float_val("lane_inset_from_tray_edge", 0.024);
  return TrayDimensions(tray_length, tray_width, fiducial_width_offset, 
                 dispense_arm_height, lane_inset);

}




