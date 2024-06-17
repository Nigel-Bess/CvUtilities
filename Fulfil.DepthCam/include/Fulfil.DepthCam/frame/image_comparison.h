//
// Created by jessv on 6/13/24.
//

#ifndef FULFIL_DISPENSE_IMAGE_COMPARISON_H
#define FULFIL_DISPENSE_IMAGE_COMPARISON_H

#include <Fulfil.DepthCam/core.h>

namespace fulfil::depthcam::image_comparison {

	cv::Scalar get_MSSIM( const cv::Mat& i1, const cv::Mat& i2);
};


#endif //FULFIL_DISPENSE_IMAGE_COMPARISON_H
