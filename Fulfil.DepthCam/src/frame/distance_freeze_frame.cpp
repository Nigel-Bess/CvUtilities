#include "Fulfil.DepthCam/frame/distance_freeze_frame.h"

#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/timer.h>
#include<librealsense2/rsutil.h>
#include <librealsense2/h/rs_types.h>
#include <stdlib.h>

using fulfil::depthcam::DistanceFreezeFrame;
using fulfil::utils::Logger;
using fulfil::utils::Timer;

DistanceFreezeFrame::DistanceFreezeFrame(std::shared_ptr<Session> session, std::shared_ptr<Eigen::Affine3d> transform,
                    int x_min, int x_max, int y_min, int y_max, float invalid_distance_data_value)
{

    this->transform = transform->cast<float>();
    this->depth_frame_intrinsics = session->get_depth_stream_intrinsics();
    this->color_frame_intrinsics = session->get_color_stream_intrinsics();
    this->color_to_depth_extrinsics = session->get_color_to_depth_extrinsics();
    this->depth_to_color_extrinsics = session->get_depth_to_color_extrinsics();

    this->invalid_distance_data_value = invalid_distance_data_value;
    int width = session->get_color_stream_intrinsics()->width;
    int height = session->get_color_stream_intrinsics()->height;
    x_max = (x_max == -1 || x_max >= width) ? width -1  : x_max;
    y_max = (y_max == -1 || y_max >= height) ? height -1 : y_max;
    x_min = (x_min < 0) ? 0 : x_min;
    y_min = (y_min < 0) ? 0 : y_min;
    this->roi = cv::Rect(cv::Point2f(x_min, y_min), cv::Size(x_max-x_min, y_max-y_min));
    this->converted = false;
    session->get_depth_mat()->copyTo(this->depth_mat);
    Logger::Instance()->Trace("Copied depthmat into freeze frame");
    this->depth_mat(roi).convertTo(this->transform_area, CV_32FC1, 0.001);
    Logger::Instance()->Trace("Clipped transformation region.");

}

float DistanceFreezeFrame::get_invalid_distance_value() const
{
  return this->invalid_distance_data_value;
}

std::shared_ptr<cv::Mat> DistanceFreezeFrame::get_local_distance_mat()
{
    std::shared_ptr<cv::Mat> local_depth = std::make_shared<cv::Mat>(this->depth_mat.size(),CV_32FC1, cv::Scalar(invalid_distance_data_value));
    if (!this->converted) {
        this->build_local_distance_mat();
        this->converted = true;
    }//TODO think about race
    cv::Mat valid_region(*local_depth, this->roi);
    this->transform_area.copyTo(valid_region);
    return local_depth;
}

cv::Point2i DistanceFreezeFrame::get_pixel_from_point(float x, float y, float z)
{
  Eigen::Vector3f eigen_point = this->transform.inverse() * Eigen::Vector3f(x,y,z);
  float depth_point[3] = {eigen_point(0),eigen_point(1),eigen_point(2)};
  float color_point[3];
  rs2_transform_point_to_point(color_point, this->depth_to_color_extrinsics.get(), depth_point);

  //Converting from color point to color pixel
  float pixel[2];
  rs2_project_point_to_pixel(pixel, this->color_frame_intrinsics.get(), color_point);
  return cv::Point2f(pixel[0], pixel[1]);
}

void DistanceFreezeFrame::transform_to_local_distance(float (&color_point)[3], int x, int y, float pixel_distance)
{
    const float raw_pixel[2] = {static_cast<float>(x), static_cast<float>(y)};
    rs2_deproject_pixel_to_point(color_point, this->color_frame_intrinsics.get(),
                                 raw_pixel, pixel_distance);

    //Converting color point to depth point
    float depth_point[3];
    rs2_transform_point_to_point(depth_point, this->color_to_depth_extrinsics.get(), color_point);

    Eigen::Map<Eigen::Matrix<float, 3, 1> > eigen_point = Eigen::Map<Eigen::Matrix<float, 3, 1> >(color_point);
    eigen_point = this->transform * eigen_point;
}


std::shared_ptr<Eigen::Vector3d> DistanceFreezeFrame::get_point_from_pixel(int x, int y)
{
    bool invalid_pixel = (this->depth_mat.at<uint16_t>(y, x) == 0);
    float pixel_distance = this->depth_mat.at<uint16_t>(y, x) * 0.001; // get the camera depth at pixel, frames aligned
    float color_point[3] = {0,0,0};
    if (!invalid_pixel)
        this->transform_to_local_distance(color_point, x, y, pixel_distance);
    return std::make_shared<Eigen::Vector3d>((double)color_point[0],(double)color_point[1],(double)color_point[2]);
}

cv::Mat DistanceFreezeFrame::build_local_distance_mat()
{
    Logger::Instance()->Debug("Region of interest extents: ({}, {}) to ({}, {}), for a total of {} spatial pixels transformed. ",
                              this->roi.tl().x,this->roi.tl().y, this->roi.br().x,this->roi.br().y, this->roi.area());

    int nRows = this->roi.height;
    int nCols = this->roi.width;
    if (this->transform_area.isContinuous())
    {
      Logger::Instance()->Debug("Using Continuous Transform!");
      nCols *= nRows;
      nRows = 1;
    }
    int i,j;
    float* p;
    float point[3] = {0,0,0};
    for( i = 0; i < nRows; ++i)
    {
        p = this->transform_area.ptr<float>(i);
        for ( j = 0; j < nCols; ++j)
        {
            if (p[j] > 0) {
              this->transform_to_local_distance(point, j % this->roi.width, j / this->roi.width, p[j]);
              p[j] = point[2];
            } else {
              p[j] = this->invalid_distance_data_value;
            }


        }
    }
    Logger::Instance()->Debug("Local Distance Matrix built!!!!!!!!");
    return this->transform_area;
}
