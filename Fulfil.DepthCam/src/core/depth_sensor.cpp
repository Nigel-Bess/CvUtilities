/**
 * This file contains the implementation of the depth sensor
 * that wraps around the realsense sensor and sdk. It also
 * hadnles all of the settings that are used for the
 * realsense depth camera.
 */
#include"depth_sensor.h"
#include<eigen3/Eigen/Geometry>
#include <memory>
#include<librealsense2/rs.hpp>
#include<librealsense2/rsutil.h>
#include <Fulfil.CPPUtils/logging.h>

using fulfil::depthcam::DepthSensor;
using fulfil::utils::Logger;

// TODO need to make frame size, rate, decimation factor configurable!!!
DepthSensor::DepthSensor(const std::string &serial)
{

    //Add desired streams to configuration
    rs2::config cfg;
    cfg.enable_device(serial.c_str());
    cfg.enable_stream(RS2_STREAM_COLOR, 1280, 720, RS2_FORMAT_BGR8, 15);
    cfg.enable_stream(RS2_STREAM_DEPTH, 1280, 720, RS2_FORMAT_Z16, 15);

    fulfil::utils::Logger::Instance()->Trace("Streams enabled on device {}.", serial);
    //Instruct pipeline to start streaming with the requested configuration
    this->profile = std::make_shared<rs2::pipeline_profile>(this->pipeline->start(cfg));
    auto depth_sensor = this->profile->get_device().first<rs2::depth_sensor>();

    if (this->profile->get_device().get_info(RS2_CAMERA_INFO_NAME) == std::string_view("Intel RealSense D457")
        && depth_sensor.supports(RS2_OPTION_EMITTER_ENABLED)) {
        depth_sensor.set_option(RS2_OPTION_EMITTER_ENABLED, 0.0F); // Disable emitter (0.0f off, 1.0f on)
        fulfil::utils::Logger::Instance()->Info("Emitter for device {} must be disabled prior to pipeline start "
          "due to firmware issues for RS D457 devices. Sensor will support emitter once Intel resolves issue.", serial);
    }
    // Transform and projection info
    this->color_intrinsics = std::make_shared<rs2_intrinsics>(
      this->profile->get_stream(RS2_STREAM_COLOR).as<rs2::video_stream_profile>().get_intrinsics());
    this->depth_intrinsics = std::make_shared<rs2_intrinsics>(this->profile
            ->get_stream(RS2_STREAM_DEPTH).as<rs2::video_stream_profile>().get_intrinsics());

    fulfil::utils::Logger::Instance()->Trace("Profile and Color intrinsics enabled on device {}.", serial);
    //Used to reduce the number of points in the point cloud.
    this->decimation_filter = std::make_shared<rs2::decimation_filter>(8);

    this->color_to_depth_extrinsics = std::make_shared<rs2_extrinsics>(
        this->profile->get_stream(RS2_STREAM_COLOR).get_extrinsics_to(this->profile->get_stream(RS2_STREAM_DEPTH)));
        
    this->depth_to_color_extrinsics = std::make_shared<rs2_extrinsics>(
        this->profile->get_stream(RS2_STREAM_DEPTH).get_extrinsics_to(this->profile->get_stream(RS2_STREAM_COLOR)));

    this->pointcloud = std::make_shared<rs2::pointcloud>();
    this->serial_number = std::make_shared<std::string>(serial);
    last_frame_time = CurrentTime();
    std::thread run(&DepthSensor::manage_pipe, this);
    run.detach();

    // WARM UP
    fulfil::utils::Logger::Instance()->Trace("WARM UP starting for camera {}.", serial);
    print_framestats();
    auto start = CurrentTime();
    while((good_frames < 30) || (ms_elapsed(start) < 1500)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    print_framestats();
    fulfil::utils::Logger::Instance()->Trace("WARM UP complete for camera {}.", serial);


}

/**
 * Helper function to abstract some of the duplicate code.
 * Takes in a frameset, a decimation filter, and returns the points from the sensor.
 * 
 * @param frames the object that contains both depth and color frames (if they are 
 *      available which they should be).
 * @param decimation_filter the filter that determines how much data from teh depth 
 *      camera is actually put into the point cloud.
 * 
 * @return a collection of 3D points in the depth sensors coordinate system.
 */
rs2::points generate_point_cloud(std::shared_ptr<rs2::depth_frame> depth_frame, std::shared_ptr<rs2::video_frame> raw_color_frame,
    std::shared_ptr<rs2::decimation_filter> decimation_filter,
    std::shared_ptr<rs2::pointcloud> point_cloud)
{
    //Get the color frames
    rs2::frame color_frame = *raw_color_frame;

    // TODO Use or remove this. map_to applies to the texture mapping -- which currently we discard
    point_cloud->map_to(color_frame);
    rs2::depth_frame decimated_depth_frame = *depth_frame;
    decimated_depth_frame = decimation_filter->process(decimated_depth_frame);
    return point_cloud->calculate(decimated_depth_frame);
}

std::shared_ptr<Eigen::Matrix3Xd> DepthSensor::get_point_cloud(std::shared_ptr<rs2::depth_frame> raw_depth_frame, std::shared_ptr<rs2::video_frame> raw_color_frame, bool include_invalid_depth_data)
{
    Logger::Instance()->Trace("Getting Point Cloud in depth_sensor #1: using get_point_cloud(std::shared_ptr<rs2::frameset> frameset, bool include_invalid_depth_data)");
    rs2::points points = generate_point_cloud(raw_depth_frame, raw_color_frame, this->decimation_filter, this->pointcloud);
    const rs2::vertex* vertices = points.get_vertices();
    if(include_invalid_depth_data)
    {
        std::shared_ptr<Eigen::Matrix3Xd> new_point_cloud = std::make_shared<Eigen::Matrix3Xd>(3, points.size());
        for(int i = 0; i < points.size(); i++)
        {
            (*new_point_cloud)(0,i) = vertices[i].x;
            (*new_point_cloud)(1,i) = vertices[i].y;
            (*new_point_cloud)(2,i) = vertices[i].z;
        }
        return new_point_cloud;
    }
    else
    {
        //Only adds the data where the depth value is not 0.
        std::shared_ptr<Eigen::Matrix3Xd> new_point_cloud =
                std::make_shared<Eigen::Matrix3Xd>(3, 0);
        int column_count = 0;
        for(int i = 0; i < points.size(); i++)
        {
            if(vertices[i].z != 0)
            {
                column_count += 1;
                new_point_cloud->conservativeResize(3, column_count);
                (*new_point_cloud)(0, column_count - 1) = vertices[i].x;
                (*new_point_cloud)(1, column_count - 1) = vertices[i].y;
                (*new_point_cloud)(2, column_count - 1) = vertices[i].z;
            }
        }
        return new_point_cloud;
    }
}

void DepthSensor::create_camera_status_msg(DepthCameras::DcCameraStatusCodes code){
    if(service_ == nullptr)return;
    DepthCameras::CameraStatusUpdate msg;
    msg.set_command_id(GetTxObjectIdString());
    msg.set_msg_type(DepthCameras::MESSAGE_TYPE_CAMERA_STATUS);
    msg.set_camera_name(name_);
    msg.set_camera_serial(*serial_number.get());
    msg.set_status_code(code);
    service_->AddStatusUpdate(msg.msg_type(), msg.SerializeAsString());
}

void DepthSensor::manage_pipe(){
    print_time = CurrentTime();
    while(true){
        auto timer = CurrentTime();
        auto success = false;
        try{
            //we are set to 15FPS which is every ~66ms
            total_frames++;
            auto new_frame = pipeline->wait_for_frames(15000);
            std::lock_guard<std::mutex> lock(_lock);
            frame_set = std::make_shared<rs2::frameset>(new_frame);
            last_frame_time = CurrentTime();
            good_frames++;
            success = true;
            if(!connected_ && name_.compare("D")){//wait for name to be assigned
                connected_ = true;
                create_camera_status_msg(DepthCameras::DcCameraStatusCodes::CAMERA_STATUS_CONNECTED);
            }

        }
        catch (const rs2::unrecoverable_error& e){
            Logger::Instance()->Fatal("{} [{}]: Unrecoverable:\nRealsense Exception {}\nIn function {}\nWith args {}",
                name_.c_str(), serial_number->c_str(), e.what(), e.get_failed_function(), e.get_failed_args());
            unrecoverable_exc++;
            create_camera_status_msg(DepthCameras::DcCameraStatusCodes::CAMERA_STATUS_NOT_RECOVERABLE_EXCEPTION);
        }
        catch (const rs2::recoverable_error& e){
            Logger::Instance()->Error("{} [{}]: Recoverable:\nRealsense Exception {}\nIn function {}\nWith args {}",
                name_.c_str(), serial_number->c_str(), e.what(), e.get_failed_function(), e.get_failed_args());
            recoverable_exc++;
            create_camera_status_msg(DepthCameras::DcCameraStatusCodes::CAMERA_STATUS_RECOVERABLE_EXCEPTION);
        }
        catch (const std::exception & e){
            Logger::Instance()->Error("{} [{}]: DepthSensor::manage_pipe exception {}", name_.c_str(), serial_number->c_str(), e.what());
            std_exceptions++;
            create_camera_status_msg(DepthCameras::DcCameraStatusCodes::CAMERA_STATUS_STD_EXCEPTION);
        }
        catch(...){
            Logger::Instance()->Error("{} [{}]: DepthSensor::manage_pipe unknown error!", name_.c_str(), serial_number->c_str());
            std_exceptions++;
            create_camera_status_msg(DepthCameras::DcCameraStatusCodes::CAMERA_STATUS_STD_EXCEPTION);
        }
        auto elapsed = ms_elapsed(timer);
        average_frame_time = average_frame_time * 0.90 + elapsed * 0.10;
        if(ms_elapsed(print_time) > 10000){
            print_framestats();
            print_time = CurrentTime();
        }
        if(!success){
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            connected_ = false;
        }
    }
}

rs2::frameset DepthSensor::get_latest_frame(){
    static std::string error("DepthSensor::get_latest_frame no good frame to return with!");
    auto start = CurrentTime();
    while(ms_elapsed(start) < 1000){
        if(frame_is_good()){
            std::lock_guard<std::mutex> lock(_lock);
            rs2::frameset frame(*frame_set.get());
            return frame;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    connected_ = false;
    create_camera_status_msg(DepthCameras::DcCameraStatusCodes::CAMERA_STATUS_NO_FRAME);
    Logger::Instance()->Error(error);
    throw (error);
}


void DepthSensor::get_fresh_frameset(cv::Mat& raw_color_mat) {
    rs2::frameset frames = get_latest_frame();
    rs2::video_frame raw_color_frame = frames.get_color_frame();
// Create OpenCV matrix of size (w,h) from the colorized depth data
    raw_color_mat = cv::Mat(cv::Size(raw_color_frame.get_width(), raw_color_frame.get_height()),
                            CV_8UC3, (void*)raw_color_frame.get_data(), cv::Mat::AUTO_STEP);
}

void DepthSensor::get_fresh_frameset(cv::Mat& raw_color_mat, cv::Mat& raw_depth_mat) {
    rs2::frameset frames = get_latest_frame();
    rs2::video_frame raw_color_frame = frames.get_color_frame();
    rs2::depth_frame raw_depth_frame = frames.get_depth_frame(); //store as independent frame before aligning frameset. Tested that it retains state after alignment.
// Create OpenCV matrix of size (w,h) from the colorized depth data
    raw_color_mat = cv::Mat(cv::Size(raw_color_frame.get_width(), raw_color_frame.get_height()),
                        CV_8UC3, (void*)raw_color_frame.get_data(), cv::Mat::AUTO_STEP);
    raw_depth_mat = cv::Mat(cv::Size(raw_depth_frame.get_width(), raw_color_frame.get_height()),
                            CV_16UC1, (void*)raw_color_frame.get_data(), cv::Mat::AUTO_STEP);

}

void DepthSensor::get_fresh_frameset(cv::Mat& raw_color_mat, cv::Mat& raw_depth_mat, cv::Mat& aligned_depth_mat) {
    rs2::frameset frames = get_latest_frame();
    rs2::video_frame raw_color_frame = frames.get_color_frame();
    rs2::depth_frame raw_depth_frame = frames.get_depth_frame(); //store as independent frame before aligning frameset. Tested that it retains state after alignment.
// Create OpenCV matrix of size (w,h) from the colorized depth data
    raw_color_mat = cv::Mat(cv::Size(raw_color_frame.get_width(), raw_color_frame.get_height()),
                            CV_8UC3, (void*)raw_color_frame.get_data(), cv::Mat::AUTO_STEP);

    raw_depth_mat = cv::Mat(cv::Size(raw_depth_frame.get_width(), raw_depth_frame.get_height()),
                            CV_16UC1, (void*)raw_depth_frame.get_data(), cv::Mat::AUTO_STEP);

    rs2::align align_to_color(RS2_STREAM_COLOR);
    rs2::depth_frame aligned_depth_frame = align_to_color.process(frames).get_depth_frame();
    aligned_depth_mat = cv::Mat(cv::Size(aligned_depth_frame.get_width(), aligned_depth_frame.get_height()),
                            CV_16UC1, (void*)aligned_depth_frame.get_data(), cv::Mat::AUTO_STEP);
}

// TODO ..... Is this a shared pointer to a raw pointer???? Holy shit fix next cycle. Leaving for now to ensure nothing breaks
void DepthSensor::get_frameset(bool align_frames, std::shared_ptr<rs2::depth_frame> *raw_depth_frame,
                               std::shared_ptr<rs2::depth_frame> *aligned_depth_frame, std::shared_ptr<rs2::video_frame> *raw_color_frame)
{
    rs2::frameset frames = get_latest_frame();

    *raw_color_frame = std::make_shared<rs2::video_frame>(frames.get_color_frame());
    *raw_depth_frame = std::make_shared<rs2::depth_frame>(frames.get_depth_frame()); //store as independent frame before aligning frameset. Tested that it retains state after alignment.

    if (align_frames)
    {
      rs2::align align_to_color(RS2_STREAM_COLOR);
      frames =  align_to_color.process(frames);
      Logger::Instance()->Trace("Got aligned depth frame!");
    }
    *aligned_depth_frame = std::make_shared<rs2::depth_frame>(frames.get_depth_frame());
}


float DepthSensor::get_depth_scale()
{
  // Go over the device's sensors
  rs2::device dev = this->profile->get_device();
  for (rs2::sensor& sensor : dev.query_sensors())
  {
    // Check if the sensor if a depth sensor
    if (rs2::depth_sensor dpt = sensor.as<rs2::depth_sensor>())
    {
      return dpt.get_depth_scale();
    }
  }
  throw std::runtime_error("Device does not have a depth sensor");
}

