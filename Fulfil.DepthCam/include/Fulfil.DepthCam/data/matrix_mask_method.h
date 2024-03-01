//
// Created by amber on 4/7/21.
//

#ifndef FULFIL_DISPENSE_MATRIX_MASK_METHOD_H
#define FULFIL_DISPENSE_MATRIX_MASK_METHOD_H

#include <memory>
#include <opencv2/opencv.hpp>

namespace fulfil
{
    namespace depthcam
    {
        namespace data
        {
            class MatrixMaskMethod
            {

            public:
                //virtual std::shared_ptr<cv::Mat> get_mask_for_distance_mat() = 0;
                //virtual std::shared_ptr<cv::Mat> get_mask_for_depth_mat() = 0;
                //virtual std::shared_ptr<cv::Mat> get_mask_for_color_mat() = 0;

                virtual void mask_distance_mat(std::shared_ptr<cv::Mat> distance_matrix) = 0;
                virtual void mask_depth_mat(std::shared_ptr<cv::Mat> depth_matrix) = 0;
                virtual void mask_color_mat(std::shared_ptr<cv::Mat> color_matrix) = 0;
                
            };
        }  // namespace 
    }  // namespace data
}  // namespace fulfil

#endif //FULFIL_DISPENSE_MATRIX_MASK_H
