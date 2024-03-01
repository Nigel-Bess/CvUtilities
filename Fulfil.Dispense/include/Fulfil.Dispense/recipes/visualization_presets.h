//
// Created by amber on 7/14/23.
//

#ifndef FULFIL_DISPENSE_VISUALIZATION_PRESETS_H
#define FULFIL_DISPENSE_VISUALIZATION_PRESETS_H


#include <array>
#include <opencv2/core/types.hpp>

namespace fulfil::recipes::visualization {
  struct LFB3ImageVisualizationSettings {
    double m_resize_factor{0.72};
    int crop_max_x {1150};
    int crop_min_x {150};
    int crop_max_y {700};
    int crop_min_y {70};
    [[nodiscard]] cv::Rect get_roi() const;
    LFB3ImageVisualizationSettings() = default;
  };
  [[nodiscard]]
  inline cv::Rect LFB3ImageVisualizationSettings::get_roi() const
  {
    return cv::Rect {cv::Point{crop_min_x, crop_min_y}, cv::Point {crop_max_x, crop_max_y}};
  }



  struct LFB3GridSettings {
    double m_resize_factor{10};
    int m_rows {8};
    int m_cols {6};
    int m_channels{3};
    LFB3GridSettings() = default;
  };
}
#endif// FULFIL_DISPENSE_VISUALIZATION_PRESETS_H
