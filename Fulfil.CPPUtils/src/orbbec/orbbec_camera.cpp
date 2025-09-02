#include <chrono>
#include <Fulfil.CPPUtils/orbbec/orbbec_camera.h>
#include "libobsensor/ObSensor.hpp"
#include <opencv2/opencv.hpp>

using namespace std::chrono_literals;
using fulfil::utils::orbbec::ColorDepthFrame;
using fulfil::utils::orbbec::OrbbecCamera;

/// Max snapshot retries till GetBlockingImage gives up taking a valid frame
const int MAX_SNAPSHOT_RETRIES = 20;

// Reset camera exposure every 4 hours to account for changing sunlight
const auto EXPOSURE_RESET_INTERVAL = 4h;

void OrbbecCamera::start_camera(){
    std::thread(&OrbbecCamera::run_camera_thread, this).detach();
}

void OrbbecCamera::kill_camera(){
    run = false;
    _connected = false;
    logger->Info("OrbbecCamera {} shutting down", _name);
    try {
        pipe->stop();
        // Reboot the camera for good measure, here we assume that
        // the camera will restart faster than this service
        device->reboot();
    } catch (...) {}
}

void OrbbecCamera::run_auto_exposure() {
    std::lock_guard<std::recursive_mutex> lock(_lifecycleLock); {
        logger->Info("run_auto_exposure for {}", _name);
        device->setBoolProperty(OB_PROP_IR_AUTO_EXPOSURE_BOOL, false);
        device->setBoolProperty(OB_PROP_IR_AUTO_EXPOSURE_BOOL, true);
        device->setBoolProperty(OB_PROP_COLOR_AUTO_EXPOSURE_BOOL, false);
        device->setBoolProperty(OB_PROP_COLOR_AUTO_EXPOSURE_BOOL, true);
        device->setBoolProperty(OB_PROP_DEPTH_AUTO_EXPOSURE_BOOL, false);
        device->setBoolProperty(OB_PROP_DEPTH_AUTO_EXPOSURE_BOOL, true);
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));//wait for auto exp to kick in
        last_exposure_reset_time = std::chrono::system_clock::now();
    }
}


// Call this whenever a steady-state ping/pong with camera succeeds to
void OrbbecCamera::log_ping_success() {
    logger->Info("Pinged {}", _name);
}

void OrbbecCamera::disconnect(std::string err_msg) {
    if (_connected) {
        logger->Error("{} Orbbec disconnected", _name, err_msg);
        pipe->stop();
        _connected = false;
        // TOOD oberserver pattern
        //AddCameraStatus(DepthCameras::DcCameraStatusCodes::CAMERA_STATUS_RECOVERABLE_EXCEPTION);
    }
}

void OrbbecCamera::run_camera_thread(){
    auto info = this->device->getDeviceInfo();
    // Log device info for easy debugging
    logger->Info("{}: Got SerialNumber: {}", _name, info->serialNumber());
    logger->Info("{}: Got Orbbec registered name: {}", _name, info->name());

    pipe = std::make_shared<ob::Pipeline>(device);
    config = std::make_shared<ob::Config>();
    // Configure which streams to enable or disable for the Pipeline by creating a Config
    config->enableVideoStream(OB_STREAM_COLOR);
    config->setAlignMode(ALIGN_D2C_SW_MODE);

    auto depthProfiles = pipe->getStreamProfileList(OB_SENSOR_DEPTH);
    auto depthProfile = depthProfiles->getVideoStreamProfile(640, 576, OB_FORMAT_Y16, 30);
    // 2. Color configuration
    auto colorProfiles = pipe->getStreamProfileList(OB_SENSOR_COLOR);
    auto colorProfile = colorProfiles->getVideoStreamProfile(1920, 1080, OB_FORMAT_RGB, 30);
    // 3. Enable Depth stream
    config->enableStream(depthProfile);
    // 4. Enable Color stream
    config->enableStream(colorProfile);

    run_auto_exposure();

    reconnect();

    while(run){
        if(!_connected) {
            std::lock_guard<std::recursive_mutex> lock(_lifecycleLock);{
                reconnect();
            }
        }
        // If connected, poll for health
        else{
            std::this_thread::sleep_for(std::chrono::milliseconds(10000));
            try {
                auto info = device->getDeviceInfo();
                // Log device info for easy debugging
                logger->Info("{}: SN: {}", _name, info->serialNumber());
                if (info->serialNumber() != serial_number) {
                    throw std::runtime_error("Serials don't match???");
                }
                this->log_ping_success();
                if ((std::chrono::system_clock::now() - last_exposure_reset_time) > EXPOSURE_RESET_INTERVAL) {
                    run_auto_exposure();
                }
            } catch (const std::exception &ex) {
                disconnect(ex.what());
            }
        } 
    }
}

void OrbbecCamera::reconnect(){
    logger->Info("OrbbecCamera::reconnect on {}", _name);
    // Start the pipeline with config
    try {
        logger->Info("Starting pipe on {}", _name);
        pipe->start(config);
        
        auto test_rgb = get_rgb_blocking();
        _connected = true;
        logger->Info("Test image success on {}, connected!", _name);
        // Write reconnection image to pod on every fresh connection
        cv::imwrite("/home/fulfil/code/Fulfil.TCS/" + _name + "_last_connection.png", *test_rgb);
    }
    catch (std::exception &e) {
        logger->Error("{} returned {} when trying to open camera", _name, e.what());
        pipe->stop();
    }
    // TODO observer pattern
    //AddCameraStatus(DepthCameras::DcCameraStatusCodes::CAMERA_STATUS_CONNECTED);
    this->log_ping_success();
}

// Save depth map in PNG format
std::shared_ptr<cv::Mat> frame_to_depth_mat( std::shared_ptr< ob::DepthFrame > depthFrame ) {
    std::vector< int > compression_params;
    compression_params.push_back( cv::IMWRITE_PNG_COMPRESSION );
    compression_params.push_back( 0 );
    compression_params.push_back( cv::IMWRITE_PNG_STRATEGY );
    compression_params.push_back( cv::IMWRITE_PNG_STRATEGY_DEFAULT );
    std::string depthName = "Depth_" + std::to_string( depthFrame->timeStamp() ) + ".png";
    const cv::Mat depth_mat( depthFrame->height(), depthFrame->width(), CV_16UC1, depthFrame->data() );
    return std::make_shared<cv::Mat>(depth_mat);
}

// Save color images in PNG format
std::shared_ptr<cv::Mat> frame_to_color_mat( std::shared_ptr< ob::ColorFrame > colorFrame ) {
    std::vector< int > compression_params;
    compression_params.push_back( cv::IMWRITE_PNG_COMPRESSION );
    compression_params.push_back( 0 );
    compression_params.push_back( cv::IMWRITE_PNG_STRATEGY );
    compression_params.push_back( cv::IMWRITE_PNG_STRATEGY_DEFAULT );
    std::string colorName = "Color_" + std::to_string( colorFrame->timeStamp() ) + ".png";
    const cv::Mat color_raw_mat( 1, colorFrame->dataSize(), CV_8UC1, colorFrame->data() );
    const cv::Mat color_mat = cv::imdecode( color_raw_mat, 1 );
    return std::make_shared<cv::Mat>(color_mat);
}

std::shared_ptr<ColorDepthFrame> OrbbecCamera::get_rgb_depth_blocking(){
    try {
        auto startTime = std::chrono::steady_clock::now();
        std::lock_guard<std::recursive_mutex> lock(_lifecycleLock); {
            auto frameSet = pipe->waitForFrames(500);
            auto raw_color_frame = frameSet->colorFrame();
            auto color_mat = frame_to_color_mat(raw_color_frame);
            auto raw_depth_frame = frameSet->depthFrame();
            auto depth_mat = frame_to_depth_mat(raw_depth_frame);

            auto stopTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime).count();
            logger->Info("OrbbecCamera::get_rgb_depth_blocking took {}ms on {}", duration, _name);
            this->log_ping_success();
            return std::make_shared<ColorDepthFrame>(color_mat, depth_mat);
        }
    } catch (std::exception &e) {
        // Assume we lost connection and trigger reconnection
        disconnect(e.what());
        throw e;
    }
}

std::shared_ptr<cv::Mat> OrbbecCamera::get_depth_blocking() {
    try {
        auto startTime = std::chrono::steady_clock::now();
        std::lock_guard<std::recursive_mutex> lock(_lifecycleLock); {
            const auto frameSet = pipe->waitForFrames(500);
            const auto raw_depth_frame = frameSet->depthFrame();
            const auto depth_mat = frame_to_depth_mat(raw_depth_frame);

            auto stopTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime).count();
            logger->Info("OrbbecCamera::get_depth_blocking took {}ms on {}", duration, _name);
            this->log_ping_success();
            return depth_mat;
        }
    } catch (std::exception &e) {
        // Assume we lost connection and trigger reconnection
        disconnect(e.what());
        throw e;
    }
}

std::shared_ptr<cv::Mat> OrbbecCamera::get_rgb_blocking() {
    try {
        auto startTime = std::chrono::steady_clock::now();
        std::lock_guard<std::recursive_mutex> lock(_lifecycleLock); {
            auto frameSet = this->pipe->waitForFrames(500);
            auto raw_color_frame = frameSet->colorFrame();
            auto color_mat = frame_to_color_mat(raw_color_frame);
            auto stopTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime).count();
            logger->Info("OrbbecCamera::get_rgb_depth_blocking took {}ms on {}", duration, _name);
            this->log_ping_success();
            return color_mat;
        }
    } catch (std::exception &e) {
        // Assume we lost connection and trigger reconnection
        disconnect(e.what());
        throw e;
    }
}
