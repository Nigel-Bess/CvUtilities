//
// Created by steve on 2/20/20.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains the implementation for generating data
 * from sessions for saving
 */
#include <fstream>
#include <json.hpp>
#include <thread>
#include <opencv2/opencv.hpp>
#include <Fulfil.CPPUtils/file_system_util.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/timer.h>
#include "Fulfil.DepthCam/coders/matrix3xd_coder.h"
#include "Fulfil.DepthCam/data/data_generator.h"
#include <Fulfil.DepthCam/core/transform_depth_session.h>
#include <Fulfil.DepthCam/point_cloud.h>
#include <filesystem>

using fulfil::utils::FileSystemUtil;
using fulfil::depthcam::data::DataGenerator;
using fulfil::utils::Logger;


DataGenerator::DataGenerator(std::shared_ptr<Session> session_to_save, std::shared_ptr<std::string> image_path_out,
        std::shared_ptr<std::string> path,
        std::shared_ptr<nlohmann::json> request_json, bool cancel_save_requests)
{
    this->session = session_to_save;
    this->destination_directory = std::make_shared<std::string>(*image_path_out);
    FileSystemUtil::join_append(this->destination_directory, *path);
    this->request_json = request_json;
}

DataGenerator::DataGenerator(std::shared_ptr<Session> session_to_save, std::shared_ptr<std::string> image_path_out,
                             std::shared_ptr<nlohmann::json> request_json, bool cancel_save_requests)
                             : session(session_to_save), destination_directory(image_path_out),
                             request_json(request_json), disable_save_call(cancel_save_requests)

{}

void DataGenerator::save_color_data(std::shared_ptr<std::string> filename)
{
    Logger::Instance()->Trace("Data Generator: save_color_data started");
    bool res = cv::imwrite(filename->c_str(), *this->session->get_color_mat());
    if (!res){
        Logger::Instance()->Error("DataGenerator failed to save color image {}", *filename);
        throw std::runtime_error("Failed to save color data");
    }
}

void DataGenerator::save_aligned_depth_data(std::shared_ptr<std::string> filename)
{
    Logger::Instance()->Trace("Data Generator: save_aligned_depth_data started");
    bool res = cv::imwrite(filename->c_str(), *this->session->get_depth_mat(true));
    if (!res){
        Logger::Instance()->Error("DataGenerator failed to save aligned depth image {}", *filename);
        throw std::runtime_error("Failed to save aligned depth data");
    }
}

void DataGenerator::save_raw_depth_data(std::shared_ptr<std::string> filename)
{
  Logger::Instance()->Trace("Data Generator: save_raw_depth_data started");
  bool res = cv::imwrite(filename->c_str(), *this->session->get_depth_mat(false));
  if (!res){
      Logger::Instance()->Error("DataGenerator failed to save raw depth image {}", *filename);
      throw std::runtime_error("Failed to save raw depth data");
  }
}

void DataGenerator::save_point_cloud(std::shared_ptr<std::string> directory_path)
{
  Logger::Instance()->Trace("Data Generator: save_point_cloud started");
  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud = session->get_point_cloud(true);
  std::shared_ptr<fulfil::depthcam::pointcloud::CameraPointCloud> camera_cloud = point_cloud->as_camera_cloud();
  std::shared_ptr<Eigen::Matrix3Xd> data = camera_cloud->get_data();
  point_cloud->encode_to_directory(directory_path);
}

// TODO for the love of god banish these pointers
void DataGenerator::save_frame_information(std::shared_ptr<std::string> frame_directory)
{
    auto timer = fulfil::utils::timing::Timer("DataGenerator::save_frame_information " + *frame_directory );

    std::shared_ptr<std::string> rgb_image_name = std::make_shared<std::string>(*frame_directory);
    FileSystemUtil::join_append(rgb_image_name, "color_image.png");
    this->save_color_data(rgb_image_name);

    this->save_point_cloud(frame_directory);

    std::shared_ptr<std::string> depth_image_name = std::make_shared<std::string>(*frame_directory);
    FileSystemUtil::join_append(depth_image_name, "aligned_depth_image.png");
    this->save_aligned_depth_data(depth_image_name);

    std::shared_ptr<std::string> depth_image_name2 = std::make_shared<std::string>(*frame_directory);
    FileSystemUtil::join_append(depth_image_name2, "raw_depth_image.png");
    this->save_raw_depth_data(depth_image_name2);

    //std::shared_ptr<std::string> depth_image_name3 = std::make_shared<std::string>(*frame_directory);
    //FileSystemUtil::join_append(depth_image_name3, "raw_depth_image.jpg");
    //this->save_raw_depth_data(depth_image_name3);
}

void DataGenerator::save_data(const std::string& file_prefix) {
    // TODO honestly too busy to keep dealing with interface overhaul. deferring for now.
    auto frame_directory = std::filesystem::path{ *destination_directory };
    frame_directory /=  file_prefix;
    do_session_save(frame_directory);
}

void DataGenerator::save_json_data(const std::string& dest_directory_name, const std::string& dest_file_name, std::shared_ptr<nlohmann::json> json_to_write)
{
    if (!json_to_write || json_to_write->is_null())   //Save the json file if one was included as input to the datagenerator (Default is " ")
    {
        Logger::Instance()->Debug("No {} is available, json file will not be saved along with data generation", dest_file_name);
    }
    else  //save json request to file for use later
    {
        std::string json_file_name = std::string(dest_directory_name);
        FileSystemUtil::join_append(json_file_name, dest_file_name);
        Logger::Instance()->Trace("JSON file saving to {}", json_file_name);
        std::ofstream file(json_file_name);
        file << json_to_write;
    }
}




void DataGenerator::save_data(const std::shared_ptr<std::string>& file_prefix)
{
  Logger::Instance()->Trace("Data Generator save_data started");
  std::shared_ptr<std::string> frame_directory = std::make_shared<std::string>(*destination_directory);
  if (file_prefix != nullptr && file_prefix->length() > 0)
    FileSystemUtil::join_append(frame_directory, *file_prefix);
  try {
    if (!FileSystemUtil::directory_exists(frame_directory->c_str())) {
      //throw std::invalid_argument("invalid data save request, destination directory did not exist");
      Logger::Instance()->Trace("Creating nested directory {}", *frame_directory);
      FileSystemUtil::create_nested_directory(frame_directory);
    }

    Logger::Instance()->Debug("Saving DataGenerator data at {} ", *frame_directory);
    this->save_json_data(*frame_directory, "json_request.json", *this->request_json);
    this->save_frame_information(frame_directory);
  } catch (const std::exception& ex) {
      Logger::Instance()->Error("Error '{}' occurred in trying to save all image + depth + pointcloud + transform data", ex.what());
  } catch (...) {
    Logger::Instance()->Error("Non-Catchable Error occurred while trying to save all image + depth + pointcloud + transform data");
    //exit(18);
  }
}

void DataGenerator::save_data()
{
  Logger::Instance()->Trace("Data Generator save_data started");
  std::shared_ptr<std::string> frame_directory = std::make_shared<std::string>(*destination_directory);
  try {
    if (!FileSystemUtil::directory_exists(frame_directory->c_str())) {
      //throw std::invalid_argument("invalid data save request, destination directory did not exist");
      Logger::Instance()->Trace("Creating nested directory {}", *frame_directory);
      FileSystemUtil::create_nested_directory(frame_directory);
    }

    Logger::Instance()->Debug("Saving DataGenerator data at {} ", *frame_directory);
    this->save_json_data(*frame_directory, "json_request.json", *this->request_json);
    this->save_frame_information(frame_directory);
  } catch (const std::exception& ex) {
      Logger::Instance()->Error("Error '{}' occurred in trying to save all image + depth + pointcloud + transform data", ex.what());
  } catch (...) {
  Logger::Instance()->Error("Non-Catchable Error occurred while trying to save all image + depth + pointcloud + transform data");
  //exit(18);
  }
}

bool fulfil::depthcam::data::DataGenerator::do_session_save(std::string frame_directory) {
    Logger::Instance()->Debug("Saving DataGenerator data at {} ", frame_directory);
    if (this->disable_save_call) {
        Logger::Instance()->Warn("DataGenerator saving was disabled. `.save_data()` is not permitted.");
        return false;
    }
    try {
        if (!FileSystemUtil::directory_exists(frame_directory.c_str())) {
            Logger::Instance()->Trace("Creating nested directory {}", frame_directory);
            FileSystemUtil::create_nested_directory(frame_directory);
        }

        this->save_json_data(frame_directory, "json_request.json", *this->request_json);
        this->save_frame_information(std::make_shared<std::string>(frame_directory));
        return true;
    } catch (const std::exception& ex) {
        Logger::Instance()->Error("Error '{}' occurred in trying to save all image + depth + pointcloud + transform data", ex.what());
    } catch (...) {
        Logger::Instance()->Error("Non-Catchable Error occurred while trying to save all image + depth + pointcloud + transform data");
        //exit(18);
    }
    return false;
}

