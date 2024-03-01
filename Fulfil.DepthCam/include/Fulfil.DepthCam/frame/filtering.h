//
// Created by amber on 5/31/22.
//

#ifndef FULFIL_DEPTHCAM_FRAME_FILTERING_H_
#define FULFIL_DEPTHCAM_FRAME_FILTERING_H_
#include <opencv2/opencv.hpp>
#include <array>

namespace fulfil::depthcam::filtering {
    cv::Mat generate_square_structuring_elem(int morph_kernel_size);

    /**
     * Will dilate and then erode a single channel matrix. Useful for hole filling a mask.
     *
     * */
    void dilate_and_erode(cv::Mat& mask, int dilation_size, int erosion_size,
                          int dilation_steps=1, int erosion_steps=1, bool close=true);


    // cv::Mat img --> BGR image
    // cv::Mat mask likely 1d
    // low/high array of hsv val
    void make_selection_by_color(const cv::Mat& img, cv::Mat& selection, std::array<uint8_t, 3>  hsv_low, std::array<uint8_t, 3>  hsv_high);

    cv::Mat make_invalid_depth_mask(const cv::Mat& img);
    cv::Mat make_invalid_depth_selection(const cv::Mat& img);

    // Assumes that Mat img is BGR,
    // Attempts to make a default guess on color filter based on BGR input
    void make_selection_by_color(const cv::Mat& img, cv::Mat& selection, uint8_t blue, uint8_t green, uint8_t red);

    void make_selection_by_color(const cv::Mat& img, cv::Mat& selection, const cv::Scalar& hsv_low, const cv::Scalar& hsv_high);
    cv::Mat make_selection_by_color(const cv::Mat& img, const cv::Scalar& hsv_low, const cv::Scalar& hsv_high);

    cv::Mat select_by_convex_poly(const cv::Mat& input_mat, std::vector<cv::Point2i> shape_vertices);
    cv::Mat make_selection_by_convex_poly(cv::Size input_mat, std::vector<cv::Point2i> shape_vertices);
    void select_by_convex_poly(cv::Mat& input_mat,std::vector<cv::Point2i> shape_vertices);

    void apply_mask(cv::Mat& input_mat, const cv::Mat& mask);
    cv::Mat create_masked_image(const cv::Mat& input_mat, const cv::Mat& mask);
    void filter_out_selection(cv::Mat& input_mat, const cv::Mat& mask);

        void merge_mask(cv::Mat& mask1,const cv::Mat& mask2);
        void merge_selection(cv::Mat& mask1,const cv::Mat& mask2);

    cv::Mat invert_matrix(const cv::Mat& mat);

    void symmetric_roi_resize(cv::Rect& roi, const cv::Size& resize, float bias_back=0.5, float bias_left=0.5);

    // Todo find a better location, generalize past vertical only
    cv::LineIterator make_lane_iterator(const cv::Point2i& front_pixel, const cv::Point2i& back_pixel,
                                        int extend_back_pixels=0, int extend_front_pixels=0);


    cv::LineIterator make_lane_iterator(const cv::Point2i& front_pixel, const cv::Point2i& back_pixel,
                                        float search_resize_scale, float back_rescale_proportion=0.5, int shift=0);
}

#endif //FULFIL_DEPTHCAM_FRAME_FILTERING_H_
