//
// Created by amber on 4/26/21.
//

#include "Fulfil.DepthCam/frame/pixel_point_converter.h"
#include<librealsense2/rsutil.h>


using fulfil::depthcam::PixelPointConverter;

/**
 *
Eigen::Affine3f transform{};
float invalid_distance_data_value = 0;
std::shared_ptr<Session> session{};
int width{1280};
int height{720};
 */


PixelPointConverter::PixelPointConverter(std::shared_ptr<Session> session,
        std::shared_ptr<Eigen::Affine3d> transform,
        float invalid_distance_data_value) : transform(transform->cast<float>()),
                                         invalid_distance_data_value(invalid_distance_data_value), session(session),
                                         width(session->get_color_stream_intrinsics()->width), height(session->get_color_stream_intrinsics()->height)
{}

PixelPointConverter::PixelPointConverter(std::shared_ptr<Session> session,
                                         std::shared_ptr<Eigen::Affine3d> transform,
                                         float invalid_distance_data_value, int height, int width) :
       transform(transform->cast<float>()),
      invalid_distance_data_value(invalid_distance_data_value), session(session),
      width(width), height(height)
{}

void PixelPointConverter::clip_to_image_dimensions(float pixel[2]) const
{
  if (pixel[0] < 0) pixel[0] = 0;
  if (pixel[0] > this->width) pixel[0] = (float)this->width;
  if (pixel[1] < 0) pixel[1] = 0;
  if (pixel[1] > this->height) pixel[1] = (float)this->height;
  //  todo change and test
  //  std::clamp(pixel[0],0.0f,(float)this->width-1);
  //  std::clamp(pixel[1],0.0f,(float)this->height-1);
}

cv::Point2i PixelPointConverter::get_pixel_from_point(const float pnt[3]) const
{
  return this->get_pixel_from_point(pnt[0], pnt[1], pnt[2]);
}
//(*this->transformation) * (*this->inner_point_cloud)

void PixelPointConverter::get_pixel_space_contour(const std::shared_ptr<Eigen::Matrix3Xd>& local_pnts,
                                                  std::vector<cv::Point2i> &pixel_space_coordinates) const
{
  pixel_space_coordinates.reserve(pixel_space_coordinates.size() + local_pnts->cols());
  for(int i = 0; i < local_pnts->cols(); i++)
    pixel_space_coordinates.emplace_back(this->get_pixel_from_point(local_pnts->col(i)(0), local_pnts->col(i)(1), local_pnts->col(i)(2)));
}

std::vector<cv::Point2i> PixelPointConverter::get_pixel_space_contour(
        const std::shared_ptr<Eigen::Matrix3Xd> &local_pnts) const
{
  std::vector<cv::Point2i> pixel_space_coordinates = std::vector<cv::Point2i>();
  pixel_space_coordinates.reserve(local_pnts->cols());
  for(int i = 0; i < local_pnts->cols(); i++){
    cv::Point2i pix = this->get_pixel_from_point(local_pnts->col(i)(0), local_pnts->col(i)(1), local_pnts->col(i)(2));
    pixel_space_coordinates.emplace_back(pix);
  }

  return pixel_space_coordinates;
}

std::vector<cv::Point2i> PixelPointConverter::get_pixel_space_contour(
        const Eigen::Matrix3Xd &local_pnts) const
{
    std::vector<cv::Point2i> pixel_space_coordinates = std::vector<cv::Point2i>();
    pixel_space_coordinates.reserve(local_pnts.cols());
    for(int i = 0; i < local_pnts.cols(); i++){
        cv::Point2i pix = this->get_pixel_from_point(local_pnts.col(i)(0), local_pnts.col(i)(1), local_pnts.col(i)(2));
        pixel_space_coordinates.emplace_back(pix);
    }
    return pixel_space_coordinates;
}


std::vector<cv::Point2i> PixelPointConverter::get_pixel_space_contour(const float *p1, const float *p2) const
{
  std::shared_ptr<Eigen::Matrix3Xd> point_coordinates = std::make_shared<Eigen::Matrix3Xd>(3,2);
  *point_coordinates << p1[0], p1[1], p1[2],
                        p2[0], p2[1], p2[2];
  return this->get_pixel_space_contour(point_coordinates);
}

cv::Point2i PixelPointConverter::get_pixel_from_point(float x, float y, float z) const
{
    Eigen::Vector3f eigen_point = this->transform.inverse() * Eigen::Vector3f(x,y,z);
    float depth_point[3] = {eigen_point(0),eigen_point(1),eigen_point(2)};
    float color_point[3];
    rs2_transform_point_to_point(color_point, this->session->get_depth_to_color_extrinsics().get(), depth_point);

    //Converting from color point to color pixel
    float pixel[2];
    rs2_project_point_to_pixel(pixel, this->session->get_color_stream_intrinsics().get(), color_point);
    this->clip_to_image_dimensions(pixel);
    return {(int)pixel[0], (int)pixel[1]};
}

Eigen::Vector3d PixelPointConverter::get_point_from_pixel(cv::Point pixel) const
{
  return get_point_from_pixel(pixel.x, pixel.y);
}


Eigen::Vector3d PixelPointConverter::get_point_from_pixel(int x, int y) const
{
  float depth = this->session->depth_at_pixel(x, y);
  float color_point[3];
  const float raw_pixel[2] = { static_cast<float>(x), static_cast<float>(y)};
  rs2_deproject_pixel_to_point(color_point, this->session->get_color_stream_intrinsics().get(), raw_pixel, depth);

  //Converting color point to depth point
  float depth_point[3];
  rs2_transform_point_to_point(depth_point, this->session->get_color_to_depth_extrinsics().get(), color_point);

  Eigen::Map<Eigen::Matrix<float, 3, 1> > eigen_point = Eigen::Map<Eigen::Matrix<float, 3, 1> >(depth_point);
  eigen_point = this->transform * eigen_point;

  return  eigen_point.cast<double>();
}

float fulfil::depthcam::PixelPointConverter::get_invalid_distance_value() const {
  return this->invalid_distance_data_value;
}


