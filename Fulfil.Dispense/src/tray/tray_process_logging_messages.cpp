//
// Created by amber on 1/18/23.
//

#include "Fulfil.Dispense/tray/tray_process_logging_messages.h"
#include <Fulfil.CPPUtils/logging.h>
using fulfil::utils::Logger;



void fulfil::dispense::tray_processing_logging::log_pixel_centers(const Eigen::Matrix3Xd &lane_center_coordinates,
    std::vector<cv::Point2i> lane_pixel_centers, int num_lanes)
{
  std::stringstream pix_coord_msg;
  pix_coord_msg.precision(4);
  for (int i = 0; i < num_lanes; i++) {
    int bi = i + num_lanes;
    pix_coord_msg << "  "<<  i << ": Front Point: (" << lane_center_coordinates.col(i).x()
                  << ", " << lane_center_coordinates.col(i).y() << ", " << lane_center_coordinates.col(i).z()
                  << " :: Back Point: (" << lane_center_coordinates.col(bi).x() << ", " << lane_center_coordinates.col(bi).y() << ", " << lane_center_coordinates.col(bi).z()
                  << "\n       |--> Front Pixel ("<< lane_pixel_centers.at(i).y <<", " << lane_pixel_centers.at(i).x <<") "
                  << " :: Back Pixel ("<< lane_pixel_centers.at(bi).y <<", " << lane_pixel_centers.at(bi).x <<")\n";
  }
  Logger::Instance()->Debug("Center points to lines for {} lane tray:\n{}.", num_lanes, pix_coord_msg.str());
}
