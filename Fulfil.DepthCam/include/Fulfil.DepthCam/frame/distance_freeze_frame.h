#ifndef FULFIL_DEPTHCAM_DISTANCE_FREEZE_FRAME_H
#define FULFIL_DEPTHCAM_DISTANCE_FREEZE_FRAME_H

#include <Fulfil.DepthCam/core/session.h>

#include <string>
#include <memory>

#include <opencv2/opencv.hpp>

namespace fulfil
{
    namespace depthcam
    {
        class LocalFrame
        {
        private:
            Eigen::Affine3d transform{};
            double min_valid_depth {0};
            double max_valid_depth {0};
            std::vector<cv::Point2i> projection{};
            Eigen::Matrix3d point_cloud{};
            cv::Mat local_frame{};


        public:
            /**
             * Constructor for the depth freeze frame.
             */
             // TODO rework to use masking class better. Sparse matrices, don't transform over image frame, instead use
             // Point cloud structure, and rescale to color size. Maybe bump decimation level?
            DistanceFreezeFrame(std::shared_ptr<Session> session,
                                  const Eigen::Affine3d& transform,
                                  double max_depth_limit,
                                  double min_depth_limit,
                                  double width, double length, std::array<double, 2> origin);

            cv::Mat get_local_frame() const;

            double get_depth_shift() const;

            double get_depth_scale() const;

            cv::Mat get_depth_mask(double max_z, double min_z) const;

            //


            cv::Point2i get_pixel_from_point(float x, float y, float z);

            std::shared_ptr<Eigen::Vector3d> get_point_from_pixel(int x, int y);

        };
    }
}

#endif // FULFIL_DEPTHCAM_DISTANCE_FREEZE_FRAME_H
