//
// Created by amber on 4/7/21.
//

#include <Fulfil.DepthCam/data/basic_depth_mask_method.h>
#include <Fulfil.CPPUtils/logging.h>

using fulfil::depthcam::data::BasicDepthMaskMethod;
using fulfil::utils::Logger;

// TODO complete rework - - non destructive, read-only view? Two versions, frame based and pointcloud based. Sparse indexing?
// TODO alternate R/W class?
BasicDepthMaskMethod::BasicDepthMaskMethod(const std::shared_ptr<cv::Mat>& distance_mat, float max_z,
        float min_z, float invalid_distance_data_value)
{
  this->distance_mask_mat = std::make_shared<cv::Mat>(distance_mat->size(),
                                                                   CV_8U, cv::Scalar(1));
  this->invalid_distance_data_value = invalid_distance_data_value;
  this->build_depth_mask(max_z, 1, distance_mat, cv::THRESH_BINARY_INV, 7);
  this->build_depth_mask(min_z, 1, distance_mat, cv::THRESH_BINARY, 7);

}

BasicDepthMaskMethod::BasicDepthMaskMethod(const std::shared_ptr<cv::Mat>& distance_mat, float max_z,
                                            float invalid_distance_data_value)
{
    this->distance_mask_mat = std::make_shared<cv::Mat>(distance_mat->size(),
                                                           CV_8U, cv::Scalar(1));
    this->invalid_distance_data_value = invalid_distance_data_value;
    this->build_depth_mask(max_z, 1, distance_mat, cv::THRESH_BINARY_INV, 7);

}

void BasicDepthMaskMethod::build_depth_mask(float clipping_distance, int set_val,
                                            const std::shared_ptr<cv::Mat>& distance_mat,
                                            cv::ThresholdTypes type, int erosion_size)
{

  cv::Mat mask = cv::Mat(distance_mat->size(), CV_8U);

  auto gen_element = [](int erosion_size)
  {
      return getStructuringElement(cv::MORPH_RECT,
                                   cv::Size(erosion_size + 1, erosion_size + 1),
                                   cv::Point(erosion_size, erosion_size));
  };
  auto erode_less = gen_element(erosion_size);
  auto erode_more = gen_element(erosion_size * 2);

  cv::threshold(*distance_mat, mask, clipping_distance, set_val, type);
  cv::dilate(mask, mask, erode_less);
  cv::erode(mask, mask, erode_more);
  this->distance_mask_mat->setTo(0, mask == 0);
  Logger::Instance()->Debug("MASK GENERATED!");
}

void BasicDepthMaskMethod::mask_distance_mat(std::shared_ptr<cv::Mat> distance_matrix)
{
  distance_matrix->setTo(this->invalid_distance_data_value, *this->distance_mask_mat == 0);
}

void BasicDepthMaskMethod::mask_depth_mat(std::shared_ptr<cv::Mat> depth_matrix)
{
  depth_matrix->setTo(0, *this->distance_mask_mat == 0);
}

void BasicDepthMaskMethod::mask_color_mat(std::shared_ptr<cv::Mat> color_matrix)
{
  color_matrix->setTo(0, *this->distance_mask_mat == 0);
}