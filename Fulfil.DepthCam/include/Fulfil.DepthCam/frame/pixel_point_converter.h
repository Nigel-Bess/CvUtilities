//
// Created by amber on 4/26/21.
//

#ifndef FULFIL_DISPENSE_LOCALPIXELPOINTCONVERTER_H
#define FULFIL_DISPENSE_LOCALPIXELPOINTCONVERTER_H

#include <Fulfil.DepthCam/core/session.h>

#include <string>
#include <memory>

#include <opencv2/opencv.hpp>

namespace fulfil
{
    namespace depthcam
    {
        class PixelPointConverter
        {
        private:
            Eigen::Affine3f transform{};
            float invalid_distance_data_value = 0;
            std::shared_ptr<Session> session{};
            int width{1280};
            int height{720};
            void clip_to_image_dimensions(float pixel[2]) const;





        protected:
            //float get_pixel_depth();


        public:
            PixelPointConverter(std::shared_ptr<Session> session,
              std::shared_ptr<Eigen::Affine3d> transform,
              float invalid_distance_data_value
            );

            PixelPointConverter(std::shared_ptr<Session> session,
                                std::shared_ptr<Eigen::Affine3d> transform,
                                float invalid_distance_data_value, int height, int width
            );

            float get_invalid_distance_value() const;

            [[nodiscard]] cv::Point2i get_pixel_from_point(float x, float y, float z) const;
            // Point in (x,y,z) order
            cv::Point2i get_pixel_from_point(const float pnt[3]) const;
            //TODO move to processing class, make identity macro
            void get_pixel_space_contour(const std::shared_ptr<Eigen::Matrix3Xd>& point_coordinates,
                                         std::vector<cv::Point2i> &pixel_space_coordinates) const;
            [[nodiscard]] std::vector<cv::Point2i> get_pixel_space_contour(
                    const std::shared_ptr<Eigen::Matrix3Xd> &local_pnts) const;
            [[nodiscard]] std::vector<cv::Point2i> get_pixel_space_contour(
                    const Eigen::Matrix3Xd &local_pnts) const;
            std::vector<cv::Point2i> get_pixel_space_contour(const float *p1, const float *p2) const;

            [[nodiscard]] Eigen::Vector3d get_point_from_pixel(int x, int y) const;
            [[nodiscard]] Eigen::Vector3d get_point_from_pixel(cv::Point pixel) const;
        };
    }
}
#endif //FULFIL_DISPENSE_PIXELPOINTCONVERTER_H
