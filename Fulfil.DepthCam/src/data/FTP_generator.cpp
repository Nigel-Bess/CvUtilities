//
// Created by steve on 2/20/20.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains the implementation for generating image frames for video
 * from sessions for saving
 */

#include "Fulfil.DepthCam/data/FTP_generator.h"
#include <Fulfil.CPPUtils/file_system_util.h>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <Fulfil.DepthCam/point_cloud.h>
#include <thread>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/curl_control.h>


using fulfil::utils::CurlController;
using fulfil::utils::FileSystemUtil;
using fulfil::utils::Logger;
using fulfil::depthcam::data::FTPGenerator;
using fulfil::utils::Logger;
using fulfil::depthcam::Session;


//TODO make an abstract uploader that handles the send, rename remote generator
// Possibly one does ftp and the other does cloud? probably only need 1 curly obj with right base dirs
FTPGenerator::FTPGenerator(std::string ftp_base, std::shared_ptr<bool> stop_video_saving)
{
    this->FTP_base = ftp_base;
    this->stop_video_saving = stop_video_saving;
}

void FTPGenerator::save_frame(std::shared_ptr<Session> session, std::string image_path, std::string image_name, float resize_factor)
{
  if (resize_factor <= 0)
    throw std::runtime_error("Attempted to resize and image with a factor less than or equal to 0 prior to upload!");
  session->lock();
  session->refresh(false); //Refreshing the RGB / depth frames but false input dictates they will not be aligned (computationally intensive)
  cv::Mat img = *session->get_color_mat();
  session->unlock();
  cv::Mat dst;

  cv::resize(img, dst, cv::Size(), resize_factor, resize_factor);
  FileSystemUtil::join_append(image_path, image_name);
  cv::imwrite(image_path, dst);
}

void FTPGenerator::save_many_frames(std::shared_ptr<Session> session, int duration, int fps, std::string local_base_path,
                                    std::string FTP_path_appendix)
{
  Logger::Instance()->Debug("Saving Video Frames in Generator!");
  int delay_time = 1000 / fps; //wait_time in ms
  int current_num = 0;
  int max_num_images = duration * fps;

  if(FileSystemUtil::directory_exists(local_base_path.c_str()))
  {
    Logger::Instance()->Info("Video folder already exists?! Returning without saving video");
    return;
  }
  else
  {
    Logger::Instance()->Trace("Creating video directory now");
    FileSystemUtil::create_nested_directory(std::make_shared<std::string>(local_base_path));
  }

  std::string complete_FTP_path = this->FTP_base;
  FileSystemUtil::join_append(complete_FTP_path, FTP_path_appendix);
  CurlController curly = CurlController(local_base_path, complete_FTP_path);

  while(current_num < max_num_images and *this->stop_video_saving == false)
  {
    std::string image_path = local_base_path;
    auto start = std::chrono::high_resolution_clock::now();
    std::chrono::system_clock::time_point timePt = start + std::chrono::milliseconds(delay_time);

    std::string image_name = "image" + std::to_string(current_num) + ".jpg";
    save_frame(session, image_path, image_name);
    curly.send_file(image_name, image_name, true);

    std::this_thread::sleep_until(timePt);
    auto end = std::chrono::high_resolution_clock::now();
    current_num += 1;
  }
  save_frame(session, local_base_path, "end.jpg"); //send end of video file
  curly.send_file("end.jpg", "end.jpg", true);
  Logger::Instance()->Debug("Video Generator Upload Complete for directory: {}", local_base_path);
}


void FTPGenerator::upload_files_from_folder(std::vector<std::string> file_names, std::string local_base_path,
                                            std::string FTP_path_appendix)
{
  std::string complete_FTP_path = this->FTP_base;
  FileSystemUtil::join_append(complete_FTP_path, FTP_path_appendix);
  CurlController curly = CurlController(local_base_path, complete_FTP_path);

  for (int index = 0; index < file_names.size(); index++)
  {
    std::string image_path = local_base_path;
    std::string image_name = file_names[index];
    FileSystemUtil::join_append(image_path, image_name);

    Logger::Instance()->Trace("Uploading visualization image from {} to FTP server", image_path);
    cv::Mat target_img = cv::imread(image_path, cv::IMREAD_COLOR);

    if(target_img.empty())
    {
      Logger::Instance()->Debug("Img not found: {}", image_path);
    }
    else
    {
      curly.send_file(image_name, image_name, true);
    }
  }
}