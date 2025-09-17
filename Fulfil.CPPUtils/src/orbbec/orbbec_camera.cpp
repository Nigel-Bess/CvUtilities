#include <chrono>
#include <Fulfil.CPPUtils/orbbec/orbbec_camera.h>
#include <Fulfil.CPPUtils/logging.h>
#include "libobsensor/ObSensor.hpp"
#include <opencv2/opencv.hpp>

using namespace std::chrono_literals;
using fulfil::utils::orbbec::ColorDepthFrame;
using fulfil::utils::orbbec::OrbbecCamera;
using fulfil::utils::Logger;

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
    logger->Info("{}: Setting up S/N {}, IP: {}, on version {}", _name, info->serialNumber(), info->getIpAddress(), info->getFirmwareVersion());
    logger->Info("{}: Got Orbbec-registered name: {}", _name, info->name());

    pipe = std::make_shared<ob::Pipeline>(device);
    config = std::make_shared<ob::Config>();

    try {
        // Get all stream profiles of the color camera, including stream resolution, frame rate, and frame format
        auto                                    colorProfiles = pipe->getStreamProfileList(OB_SENSOR_COLOR);
        std::shared_ptr<ob::VideoStreamProfile> colorProfile  = nullptr;
        if(colorProfiles) {
            colorProfile = std::const_pointer_cast<ob::StreamProfile>(colorProfiles->getProfile(OB_PROFILE_DEFAULT))->as<ob::VideoStreamProfile>();
        }
        else {
            logger->Info("No color profiles!!!!");
            exit(1);
        }
        config->enableStream(colorProfile);
    }
    catch(ob::Error &e) {
        // no Color Sensor
        std::cerr << "Current device is not support color sensor!" << std::endl;
    }

    // Get all stream profiles of the depth camera, including stream resolution, frame rate, and frame format
    auto                                    depthProfiles = pipe->getStreamProfileList(OB_SENSOR_DEPTH);
    std::shared_ptr<ob::VideoStreamProfile> depthProfile  = nullptr;
    if(depthProfiles) {
        depthProfile = std::const_pointer_cast<ob::StreamProfile>(depthProfiles->getProfile(OB_PROFILE_DEFAULT))->as<ob::VideoStreamProfile>();
    }
    config->enableStream(depthProfile);

    // Configure which streams to enable or disable for the Pipeline by creating a Config
    //config->enableVideoStream(OB_STREAM_COLOR, 640, 360, 5, OB_FORMAT_RGB);
    //config->setAlignMode(ALIGN_D2C_SW_MODE);
    //config->setFrameAggregateOutputMode(OB_FRAME_AGGREGATE_OUTPUT_ALL_TYPE_FRAME_REQUIRE);

    // Depth configuration
    //auto depthProfiles = pipe->getStreamProfileList(OB_SENSOR_DEPTH);
    //auto depthProfile = depthProfiles->getVideoStreamProfile(320, 200, OB_FORMAT_Y16, 5);
    //config->enableStream(depthProfile);
    //logger->Info("{}: depth set", _name);

    // Color configuration
    //auto colorProfiles = pipe->getStreamProfileList(OB_SENSOR_COLOR);

    // Largest
    //auto colorProfile = colorProfiles->getVideoStreamProfile(1280, 800, OB_FORMAT_RGB, 5);
    // Smallest (test only)
    //auto colorProfile = colorProfiles->getVideoStreamProfile(640, 360, OB_FORMAT_RGB, 5);
    //config->enableStream(colorProfile);

    run_auto_exposure();
    logger->Info("autoexp", _name);

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
    // Start the pipe with config
    try {
        logger->Info("Starting pipe on {}", _name);
        pipe->start(config);
        logger->Info("Started pipe on {}", _name);
        
        auto test_rgb = get_rgb_blocking();
        if (!test_rgb.empty()) {
            logger->Info("Saving debug image for {} to /home/fulfil/data/debug", _name);
            cv::imwrite("/home/fulfil/data/debug/" + _name + ".png", test_rgb);
            logger->Info("Saved debug image for {} to /home/fulfil/data/debug", _name);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            throw std::runtime_error("Image was empty??? " + _name);
        }
        _connected = true;
        logger->Info("Test image success on {}, connected!", _name);
        // Write reconnection image to pod on every fresh connection
        cv::imwrite("/home/fulfil/code/Fulfil.TCS/" + _name + "_last_connection.png", test_rgb);
    }
    catch (std::exception &e) {
        logger->Error("{} returned {} when trying to open camera", _name, e.what());
        pipe->stop();
        reconnect();
    }
    // TODO observer pattern
    //AddCameraStatus(DepthCameras::DcCameraStatusCodes::CAMERA_STATUS_CONNECTED);
    this->log_ping_success();
}

// Get depth map in PNG format
cv::Mat frame_to_depth_mat( std::shared_ptr< ob::DepthFrame > depthFrame ) {
    std::vector< int > compression_params;
    compression_params.push_back( cv::IMWRITE_PNG_COMPRESSION );
    compression_params.push_back( 0 );
    compression_params.push_back( cv::IMWRITE_PNG_STRATEGY );
    compression_params.push_back( cv::IMWRITE_PNG_STRATEGY_DEFAULT );
    std::string depthName = "Depth_" + std::to_string( depthFrame->timeStamp() ) + ".png";
    const cv::Mat depth_mat( depthFrame->height(), depthFrame->width(), CV_16UC1, depthFrame->data() );
    return depth_mat;
}

// Get color images in PNG format

cv::Mat frame_to_color_mat( std::shared_ptr< ob::ColorFrame > colorFrame ) {
    ob::FormatConvertFilter formatConverFilter;
    formatConverFilter.setFormatConvertType(FORMAT_MJPEG_TO_RGB888);
   Logger::Instance()->Info("1");

    formatConverFilter.setFormatConvertType(FORMAT_RGB_TO_BGR);
   Logger::Instance()->Info("1.5");
    auto pColorFrame = formatConverFilter.process(colorFrame)->as<ob::ColorFrame>();
   std::cout<< "2";
   Logger::Instance()->Info("2");
    //pColorFrame = formatConverFilter.process(pColorFrame)->as<ob::ColorFrame>();
   std::cout<< "3";
   Logger::Instance()->Info("3");


    const cv::Mat color_raw_mat( 1, pColorFrame->dataSize(), CV_8UC1, pColorFrame->data() );
   std::cout<< "4";
   Logger::Instance()->Info("4");
    const cv::Mat color_mat = cv::imdecode( color_raw_mat, 1 );
   std::cout<< "65";
    return color_mat;
}

std::shared_ptr<ColorDepthFrame> OrbbecCamera::get_rgb_depth_blocking(){
    try {
        std::lock_guard<std::recursive_mutex> lock(_lifecycleLock); {
            auto startTime = std::chrono::steady_clock::now();
            auto frameSet = pipe->waitForFrameset(10000);
            if (frameSet == nullptr) {
                logger->Error("OrbbecCamera::get_rgb_depth_blocking got null frameset! {}", _name);
                throw std::runtime_error("pipe->waitForFrames returned null!");
            }
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

cv::Mat OrbbecCamera::get_depth_blocking() {
    try {
        std::lock_guard<std::recursive_mutex> lock(_lifecycleLock); {
            auto startTime = std::chrono::steady_clock::now();
            const auto frameSet = pipe->waitForFrameset(10000);
            if (frameSet == nullptr) {
                logger->Error("OrbbecCamera::get_depth_blocking got null frameset! {}", _name);
                throw std::runtime_error("pipe->waitForFrames returned null!");
            }
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

void saveColor( std::shared_ptr< ob::ColorFrame > colorFrame ) {
    std::vector< int > compression_params;
    compression_params.push_back( cv::IMWRITE_PNG_COMPRESSION );
    compression_params.push_back( 0 );
    compression_params.push_back( cv::IMWRITE_PNG_STRATEGY );
    compression_params.push_back( cv::IMWRITE_PNG_STRATEGY_DEFAULT );
    std::string colorName = "Color_" + std::to_string( colorFrame->timeStamp() ) + ".png";
    cv::Mat     colorRawMat( 1, colorFrame->dataSize(), CV_8UC1, colorFrame->data() );
    cv::Mat     colorMat = cv::imdecode( colorRawMat, 1 );
    cv::imwrite( colorName, colorMat, compression_params );
    std::cout << "Color saved:" << colorName << std::endl;
}

cv::Mat OrbbecCamera::get_rgb_blocking() {
    try {
        std::lock_guard<std::recursive_mutex> lock(_lifecycleLock); {
            auto startTime = std::chrono::steady_clock::now();
            logger->Info("waiting... {}", _name);
            auto frameSet = this->pipe->waitForFrameset(10000);
            if (frameSet == nullptr) {
                logger->Error("OrbbecCamera::get_rgb_blocking got null frameset! {}", _name);
                throw std::runtime_error("pipe->waitForFrames returned null!");
            }

            auto stopTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime).count();
            logger->Info("OrbbecCamera::get_rgb_depth_blocking took {}ms on {}", duration, _name);
            logger->Info("waited for non-null frameset... {}", _name);
            auto raw_color_frame = frameSet->colorFrame();
            
            if (raw_color_frame == nullptr) {
                logger->Error("OrbbecCamera::get_rgb_blocking got null color frame! {}", _name);
                throw std::runtime_error("pipe->waitForFrames returned null!");
            }

            Logger::Instance()->Info("got color frame");
            Logger::Instance()->Info("OrbbecCamera::get_rgb_depth_blocking got some pixels size={}", raw_color_frame->dataSize());
            auto data = raw_color_frame->data();

            logger->Info("callinggggg");

            cv::Mat colorRawMat( 1, raw_color_frame->dataSize(), CV_8UC1, raw_color_frame->data() );
            cv::Mat color_mat = cv::imdecode( colorRawMat, 1 );
            if (color_mat.empty()) {
                logger->Info("got color mat1 {}, isEmpty: {}", _name, color_mat.empty());
            } else {
                logger->Info("Good 1!!!! {}, isEmpty: {}", _name, color_mat.empty());
                cv::imwrite("/home/fulfil/data/test.png", color_mat);
                logger->Info("Wrote 1!!!! {}, isEmpty: {}", _name, color_mat.empty());
            }

            cv::Mat colorRawMat2( 1, raw_color_frame->dataSize(), CV_8UC1, raw_color_frame->data() );
            cv::Mat color_mat2 = cv::imdecode( colorRawMat2, 1 );
            if (color_mat2.empty()) {
                logger->Info("got color mat2 {}, isEmpty: {}", _name, color_mat2.empty());
            } else {
                logger->Info("Good 2!!!! {}, isEmpty: {}", _name, color_mat.empty());
            }

            auto color = frame_to_color_mat(raw_color_frame);
            logger->Info("got color mat {}, isEmpty: {}", _name, color.empty());
            exit(1);
            this->log_ping_success();
            return color;
        }
    } catch (std::exception &e) {
        // Assume we lost connection and trigger reconnection

        // TODO: remove this reboot, only for early debugging!
        //logger->Info("OrbbecCamera::get_rgb_depth_blocking failed, rebooting {}", _name);
        //this->device->reboot();
        disconnect(e.what());
        throw e;
    }
}
