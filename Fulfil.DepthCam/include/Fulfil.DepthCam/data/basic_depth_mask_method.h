//
// Created by amber on 4/7/21.
//

#ifndef FULFIL_DISPENSE_BASIC_DEPTH_MASK_METHOD_H
#define FULFIL_DISPENSE_BASIC_DEPTH_MASK_METHOD_H

#include <Fulfil.DepthCam/data/matrix_mask_method.h>

namespace fulfil
{
    namespace depthcam
    {
        namespace data
        {
            class BasicDepthMaskMethod : public MatrixMaskMethod
            {
            private:
                std::shared_ptr<cv::Mat> distance_mask_mat;
                float invalid_distance_data_value;
                void build_depth_mask(float clipping_distance, int set_val, const std::shared_ptr<cv::Mat>& distance_mat,
                                 cv::ThresholdTypes type,
                                 int erosion_size);

            public:

                BasicDepthMaskMethod(const std::shared_ptr<cv::Mat>& distance_mat, float max_z,
                        float invalid_distance_data_value);

                BasicDepthMaskMethod(const std::shared_ptr<cv::Mat>& distance_mat, float max_z, float min_z,
                                     float invalid_distance_data_value);

               void mask_distance_mat(std::shared_ptr<cv::Mat> distance_matrix) override;
               void mask_depth_mat(std::shared_ptr<cv::Mat> depth_matrix) override;
               void mask_color_mat(std::shared_ptr<cv::Mat> color_matrix) override;


            };
        }  // namespace 
    }  // namespace data
}  // namespace fulfil

#endif //FULFIL_DISPENSE_BASIC_DEPTH_MASK_METHOD_H
