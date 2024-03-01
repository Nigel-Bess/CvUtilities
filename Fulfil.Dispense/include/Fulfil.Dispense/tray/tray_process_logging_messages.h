//
// Created by amber on 1/18/23.
//

#ifndef FULFIL_DISPENSE_TRAY_PROCESS_LOGGING_MESSAGES_H
#define FULFIL_DISPENSE_TRAY_PROCESS_LOGGING_MESSAGES_H
#include <Fulfil.CPPUtils/eigen.h>
#include <Fulfil.DepthCam/visualization.h>
#include <Fulfil.DepthCam/frame/pixel_point_converter.h>


namespace fulfil::dispense::tray_processing_logging {
void log_pixel_centers(const Eigen::Matrix3Xd &lane_center_coordinates,
    std::vector<cv::Point2i> lane_pixel_centers,
    int num_lanes);
}
#endif// FULFIL_DISPENSE_TRAY_PROCESS_LOGGING_MESSAGES_H
