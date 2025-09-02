#ifndef FULFIL_ORBBEC_UTILS_H
#define FULFIL_ORBBEC_UTILS_H

#include <thread>
#include <opencv2/opencv.hpp>
#include <filesystem>

#include <Fulfil.CPPUtils/logging.h>
#include <iostream>
#include "libobsensor/ObSensor.hpp"

namespace fulfil
{
    namespace utils
    {
        namespace orbbec
        {
            struct ColorDepthFrame
            {
                ColorDepthFrame(std::shared_ptr<cv::Mat> color_img, std::shared_ptr<cv::Mat> depth_img) : color_img(color_img), depth_img(depth_img) {};
                std::shared_ptr<cv::Mat> color_img;
                std::shared_ptr<cv::Mat> depth_img;
            };

            class OrbbecObserver {
            public:
                virtual void handle_connection_change(std::string cam_name, bool currentlyConnected);
            };

            class OrbbecCamera
            {
            public:
                OrbbecCamera(std::string _name, std::shared_ptr<ob::Device> device, fulfil::utils::Logger *logger): _name(_name), device(device), logger(logger) {};
                std::shared_ptr<cv::Mat> get_rgb_blocking();
                std::shared_ptr<cv::Mat> get_depth_blocking();
                std::shared_ptr<ColorDepthFrame> get_rgb_depth_blocking();
                void start_camera();
                void run_auto_exposure();
                void kill_camera();

                inline bool connected() { return _connected; }
                inline std::string name() { return _name; }

                /**
                 * Register an observer that's called whenever camera connection status changes
                 */
                void add_connected_callback(std::string name, OrbbecObserver* observer);

            private:
                void run_camera_thread();
                void reconnect();
                void log_ping_success();
                void disconnect(std::string err_msg);

                volatile bool run = true;
                volatile bool _connected = false;
                std::chrono::_V2::system_clock::time_point last_exposure_reset_time = std::chrono::system_clock::now();
                std::recursive_mutex _lifecycleLock;
                fulfil::utils::Logger *logger;
                std::string _name;
                std::string serial_number;
                std::shared_ptr<ob::Device> device;
                std::shared_ptr<ob::Pipeline> pipe;
                std::shared_ptr<ob::Config> config;
            };
        }
    }
}

#endif