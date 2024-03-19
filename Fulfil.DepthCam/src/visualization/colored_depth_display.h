//
// Created by nkaffine on 12/2/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file outlines the functionality of the helper
 * to determine the color of a point based on the depth
 * value at that point.
 */
#ifndef COLORED_DEPTH_DISPLAY_H_
#define COLORED_DEPTH_DISPLAY_H_
#include <Fulfil.DepthCam/core/session.h>
#include <opencv2/opencv.hpp>
#include <eigen3/Eigen/Geometry>
#include <memory>

namespace fulfil::depthcam::visualization
{
    class ColoredDepthDisplay
    {
        private:
            /**
            * The minimum recorded depth from the point cloud
            * being displayed.
            */
            float min_depth;
            /**
            * The maximum recorded depth from the point cloud
            * being displayed.
            */
            float max_depth;
            /**
            * The size of one step in depth that is equal to one step
            * in rgb color (rgb step is +-1 on the 0-255 scale for
            * red and blue).
            */
            float step;
        public:
            /**
            * ColorDepthDisplay Constructor
            * @param min_depth minimum depth value being displayed.
            * @param max_depth maximum depth value being displayed.
            */
            ColoredDepthDisplay(float min_depth, float max_depth);
            /**
            * Returns a scalar for the color at the given depth.
            * @param depth recorded by the depth sensor.
            * @return scalar representative of that depth.
            */
            cv::Scalar color_at_depth(float depth);
    };
} // namespace fulfil::depthcam::visualization
#endif