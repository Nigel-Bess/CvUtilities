//
// Created by amber on 5/31/22.
//
#include "Fulfil.DepthCam/frame/filtering.h"
#include <cmath>


cv::Mat fulfil::depthcam::filtering::generate_square_structuring_elem(int morph_kernel_size){
  auto odd_kernel_size = [](int kernel_size) { return (kernel_size % 2 == 0) ? kernel_size + 1 : kernel_size; };
  auto gen_element = [](int kern_size)
  {
    return getStructuringElement(cv::MORPH_RECT, cv::Size(kern_size, kern_size));
  };
  return gen_element(odd_kernel_size(morph_kernel_size));
}


//TODO have the users supply a vector of morph lambdas, may break our gen structuring elem into its own function
void fulfil::depthcam::filtering::dilate_and_erode(cv::Mat& mask, int dilation_size, int erosion_size, int dilation_steps, int erosion_steps, bool close) {
    auto _dilate = [&](auto struct_elem, int iters) { cv::dilate(mask, mask, struct_elem, cv::Point(-1,-1), iters) ; };
    auto _erode =  [&](auto struct_elem, int iters) { cv::erode(mask,  mask, struct_elem, cv::Point(-1,-1), iters) ; };

    auto do_morph = [&](auto morph_fn, int kern_sz, int steps) {
        if (kern_sz > 0 && steps > 0){
            auto elem = generate_square_structuring_elem(kern_sz);
            morph_fn(elem, steps); }
    };
    // Closing is a dilation followed by an erosion, which fills holes within selections
    if (close) {
      do_morph(_dilate, dilation_size, dilation_steps);
      do_morph(_erode, erosion_size, erosion_steps);
    } else {
      // Opening is an erosion followed by a dilation, which breaks weak connections between selection sections
      do_morph(_erode, erosion_size, erosion_steps);
      do_morph(_dilate, dilation_size, dilation_steps);
    }

}

cv::Mat
fulfil::depthcam::filtering::make_invalid_depth_mask(const cv::Mat& img) {
    cv::Mat invalid_depth_mask;
    cv::compare(img, 0, invalid_depth_mask, cv::CMP_GT);
    return invalid_depth_mask;
}

cv::Mat
    fulfil::depthcam::filtering::make_invalid_depth_selection(const cv::Mat& img) {
  cv::Mat invalid_depth_selection;
  cv::compare(img, 0, invalid_depth_selection, cv::CMP_EQ);
  return invalid_depth_selection;
}

void fulfil::depthcam::filtering::make_selection_by_color(const cv::Mat& img, cv::Mat& selection, uint8_t blue, uint8_t green, uint8_t red) {
    auto color = cv::Scalar(blue, green, red);
    cv::cvtColor(color, color, cv::COLOR_BGR2HSV);
    auto low_hsv =  cv::Scalar(color[0]-10, 100, 100);
    auto high_hsv = cv::Scalar(color[0]+10, 255, 255);
    cv::Mat hsv_image;
    cv::cvtColor(img, hsv_image, cv::COLOR_BGR2HSV); // This may be double copies
    cv::inRange(hsv_image, low_hsv, high_hsv, selection);
}

void fulfil::depthcam::filtering::make_selection_by_color(const cv::Mat& img,
      cv::Mat& selection, std::array<uint8_t, 3>  hsv_low, std::array<uint8_t, 3>  hsv_high)
{
    auto to_scalar = [](std::array<uint8_t, 3>  col) { return cv::Scalar(col[0],col[1], col[2]); };
    cv::Mat hsv_image;
    cv::cvtColor(img, hsv_image, cv::COLOR_BGR2HSV); // This may be double copy
    cv::inRange(hsv_image, to_scalar(hsv_low), to_scalar(hsv_high), selection);
}


void fulfil::depthcam::filtering::make_selection_by_color(const cv::Mat& img, cv::Mat& selection, const cv::Scalar& hsv_low, const cv::Scalar& hsv_high)
{
    cv::Mat hsv_image;
    cv::cvtColor(img, hsv_image, cv::COLOR_BGR2HSV); // This may be double copy
    cv::inRange(hsv_image, hsv_low, hsv_high, selection);
}

cv::Mat fulfil::depthcam::filtering::make_selection_by_color(const cv::Mat& img, const cv::Scalar& hsv_low, const cv::Scalar& hsv_high)
{
  cv::Mat selection;
  cv::Mat hsv_image;
  cv::cvtColor(img, hsv_image, cv::COLOR_BGR2HSV); // This may be double copy
  cv::inRange(hsv_image, hsv_low, hsv_high, selection);
  return selection;
}

void fulfil::depthcam::filtering::select_by_convex_poly(cv::Mat& input_mat,
                                                           std::vector<cv::Point2i> shape_vertices) {
  cv::Mat mask = cv::Mat(input_mat.size(), CV_8UC1, cv::Scalar(255, 255, 255));
  cv::fillConvexPoly(mask, &shape_vertices[0], (int)shape_vertices.size(),
                     cv::Scalar(0,0,0), 8, 0);
  input_mat.setTo(cv::Scalar::all(0), mask);
}

cv::Mat fulfil::depthcam::filtering::select_by_convex_poly(const cv::Mat& input_mat,
                                std::vector<cv::Point2i> shape_vertices) {
  CV_Assert(input_mat.type()==CV_8UC3 ||input_mat.type()==CV_8UC1 );
  cv::Mat output_mat = cv::Mat::zeros(input_mat.size(), input_mat.type());
  cv::Mat mask = cv::Mat::zeros(input_mat.size(), CV_8UC1);
  cv::fillConvexPoly(mask, &shape_vertices[0], (int)shape_vertices.size(),
                     cv::Scalar(255, 255, 255), 8, 0);
  bitwise_and(input_mat, input_mat, output_mat, mask);
  return output_mat;
}

cv::Mat fulfil::depthcam::filtering::create_masked_image(const cv::Mat& input_mat, const cv::Mat& mask) {
    cv::Mat output_mat = cv::Mat::zeros(input_mat.size(), input_mat.type());
    if (input_mat.type()==CV_8UC3 ||input_mat.type()==CV_8UC1 ) {
        bitwise_and(input_mat, input_mat, output_mat, mask);
        return output_mat;
    }
    input_mat.copyTo(output_mat, mask);
    return output_mat;
}

void fulfil::depthcam::filtering::apply_mask(cv::Mat& input_mat, const cv::Mat& mask) {
    if (input_mat.type()==CV_8UC3 ||input_mat.type()==CV_8UC1 ) {
        cv::bitwise_and(input_mat, input_mat, input_mat, mask); }
    cv::bitwise_not(mask, mask);
    input_mat.setTo(0, mask);
    cv::bitwise_not(mask, mask);
}

void fulfil::depthcam::filtering::filter_out_selection(cv::Mat& input_mat, const cv::Mat& mask) {
    input_mat.setTo(cv::Scalar::all(0), mask);
}


void fulfil::depthcam::filtering::merge_mask(cv::Mat& mask1, const cv::Mat& mask2) {
  CV_Assert(mask2.type()==CV_8UC1 && mask1.type()==CV_8UC1 );
    cv::bitwise_and(mask1, mask2, mask1);
}

void fulfil::depthcam::filtering::merge_selection(cv::Mat& mask1, const cv::Mat& mask2) {
  CV_Assert(mask2.type()==CV_8UC1 && mask1.type()==CV_8UC1 );
  cv::bitwise_or(mask1, mask2, mask1);
}


cv::Mat fulfil::depthcam::filtering::make_selection_by_convex_poly(cv::Size mask_img_size,
                                       std::vector<cv::Point2i> shape_vertices) {
  cv::Mat mask = cv::Mat::zeros(mask_img_size, CV_8UC1);
  cv::fillConvexPoly(mask, &shape_vertices[0], (int)shape_vertices.size(),
                     cv::Scalar(255, 255, 255), 8, 0);
  return mask;
}


cv::Mat fulfil::depthcam::filtering::invert_matrix(const cv::Mat& mat) {
    cv::Mat inverted;
    cv::bitwise_not(mat, inverted);
    return inverted;
}

// Size is (width, height)
void fulfil::depthcam::filtering::symmetric_roi_resize(cv::Rect& roi, const cv::Size& resize, float bias_back, float bias_left) {
    roi += resize;
    float new_tl_x = resize.width*bias_left;  float new_tl_y = resize.height*bias_back;
    if (new_tl_x < 0) { new_tl_x = 0; }
    if (new_tl_y < 0) { new_tl_y = 0; }
    roi -= cv::Point(std::round(new_tl_x), std::round(new_tl_y));
}

cv::LineIterator fulfil::depthcam::filtering::make_lane_iterator(const cv::Point2i& front_pixel, const cv::Point2i& back_pixel,
                                                   int extend_back_pixels, int extend_front_pixels)
{
    //TODO clean
    cv::Rect search_region = cv::Rect(front_pixel, back_pixel);
    int change_in_length =  extend_back_pixels + extend_front_pixels;
    float weight_of_vertical_shift_back = change_in_length == 0 ? 0.5 :
                                          static_cast<float>(extend_back_pixels)/static_cast<float>(change_in_length);
    // Note that the ROI clips the iterator boundaries.
    // So it is important that the width can cover the horizontal extent of the line (which can be nearly vertical)
    constexpr int search_region_width_buffer = 20;
    fulfil::depthcam::filtering::symmetric_roi_resize(search_region, cv::Size(search_region_width_buffer,
                                      extend_back_pixels+extend_front_pixels), weight_of_vertical_shift_back);
    return cv::LineIterator(search_region, front_pixel, back_pixel, 8, false);
}



cv::LineIterator fulfil::depthcam::filtering::make_lane_iterator(const cv::Point2i& front_pixel, const cv::Point2i& back_pixel,
                                                   float search_resize_scale, float back_rescale_proportion, int shift)
{
    //auto color_filter_zone = cv::Rect(line_front, line_back);
    cv::Rect search_region = cv::Rect(front_pixel, back_pixel);
    constexpr int search_region_width_buffer = 20;
    fulfil::depthcam::filtering::symmetric_roi_resize(search_region, cv::Size(search_region_width_buffer,
            std::round(search_resize_scale*search_region.height)), back_rescale_proportion);
    search_region += cv::Point2i(0, shift);
    cv::LineIterator search_path = cv::LineIterator(search_region, front_pixel, back_pixel, 8, false);
    return search_path;

}
