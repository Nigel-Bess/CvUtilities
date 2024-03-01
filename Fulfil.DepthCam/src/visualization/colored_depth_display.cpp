/**
 * This file includes the implementation of a helper to determine
 * a color associated with depth data being displayed.
 */
#include "colored_depth_display.h"
using fulfil::depthcam::visualization::ColoredDepthDisplay;

ColoredDepthDisplay::ColoredDepthDisplay(float min_depth, float max_depth)
{
    this->min_depth = min_depth;
    this->max_depth = max_depth;
    this->step = (max_depth - min_depth) / (512);
}

cv::Scalar ColoredDepthDisplay::color_at_depth(float depth)
{
    int color_step = (int)((depth - min_depth) / this->step);
    return cv::Scalar(512-color_step, 0, color_step);
}