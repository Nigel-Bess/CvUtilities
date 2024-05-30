//
// Created by Amber Thomas on 7/29/21.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains the implementation for generating image frames for video
 * from sessions for saving
 */
#include "Fulfil.DepthCam/data/upload_generator.h"
#include "Fulfil.DepthCam/coders/extrinsics_coder.h"
#include <Fulfil.CPPUtils/file_system_util.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/timer.h>
#include <Fulfil.DepthCam/core/transform_depth_session.h>
#include <Fulfil.DepthCam/data/file_sender.h>
#include <Fulfil.DepthCam/point_cloud.h>
#include <cstdlib>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <cstdio>
#include <sys/stat.h>
#include <thread>


using fulfil::depthcam::data::FileSender;
using fulfil::utils::FileSystemUtil;
using fulfil::utils::Logger;
using fulfil::depthcam::data::UploadGenerator;
using fulfil::utils::Logger;
using fulfil::depthcam::Session;



//TODO make an abstract uploader that handles the send, rename remote generator
// Possibly one does ftp and the other does cloud? probably only need 1 curly obj with right base dirs
UploadGenerator::UploadGenerator(int default_stop_video_delay) : stop_video_delay(stop_video_delay) {}

void UploadGenerator::save_frame(std::shared_ptr<fulfil::depthcam::Session> session, std::string image_path,
                                 std::string image_name, float resize_factor)
{
  //Refreshing the RGB / depth frames but false inputs indicates will bypass aligning and validation checks
  // NOTE: it is INCREDIBLY IMPORTANT to ignore validation checks for videos because sometimes mechanical components will block
  // the camera during videos and will result in blank depth frames!!!
  cv::Mat dst;
  //cv::Mat img = *session->get_color_mat();
  cv::resize(session->grab_color_frame(), dst, cv::Size(), resize_factor, resize_factor);
  //cv::resize(img, dst, cv::Size(), resize_factor, resize_factor);
  cv::putText(dst,
    fulfil::utils::timing::return_current_time_ms_and_date(), cv::Point((int)dst.cols*0.12, dst.rows - 10),
      cv::FONT_HERSHEY_SIMPLEX, 0.35, cv::Scalar(255), 1);
  FileSystemUtil::join_append(image_path, image_name);
  cv::imwrite(image_path, dst);
}

int  UploadGenerator::generate_video(std::string video_file_path, std::string video_file_name, std::string name_fmt, int fps)
{
    char cmd_cstr[1028];

    std::string full_image_fmt = video_file_path; // full_image_fmt =  video_file_path/name_fmt
    FileSystemUtil::join_append(video_file_path, video_file_name); //video_file_path/video_file_name
    FileSystemUtil::join_append(full_image_fmt, name_fmt);
    Logger::Instance()->Debug("Generating {} from images {}.", video_file_path, full_image_fmt);
    sprintf(cmd_cstr, "ffmpeg -framerate %d -i %s -pix_fmt yuv444p %s > /dev/null 2>&1", fps, full_image_fmt.c_str(), video_file_path.c_str());

    int video_gen_exit = std::system(cmd_cstr);
    if (video_gen_exit != 0){
        Logger::Instance()->Error("Error occurred while generating mp4 with code {}. Cancelling upload! "
          "Command sent to ffmpeg:\n    {}", video_gen_exit, cmd_cstr);
        //throw std::runtime_error("Error creating mp4 with ffmpeg in upload generator!");
    }
    struct stat buffer;
    if (stat(video_file_path.c_str(), &buffer)!=0){
      // probably don't want to throw tbh, but better than exiting
        Logger::Instance()->Error("mp4 file does not exist at {}. Cancelling upload", video_file_path);
        video_gen_exit = -1;
        //throw std::runtime_error("Upload generator failed to create mp4 file in upload generator!");
    }
    return video_gen_exit;
}

void UploadGenerator::save_many_frames(std::shared_ptr<fulfil::depthcam::Session> session, std::shared_ptr<FileSender> sender,  int duration, int fps,
                                       std::string local_base_path, std::string remote_base_path, bool send_frames, bool send_mp4)
{

    fps = std::clamp(fps, 1, 15);
  Logger::Instance()->Debug("Saving Video Frames in Generator at {} fps!", fps);
  int delay_time = 1000 / fps; //wait_time in ms
  int current_num = 0;
  int max_num_images = duration * fps;

  if(FileSystemUtil::directory_exists(local_base_path.c_str()))
  {
    Logger::Instance()->Error("Video folder already exists?! Returning without saving video");
    return;
  }
  else
  {
    Logger::Instance()->Trace("Creating video directory now");
    FileSystemUtil::create_nested_directory(std::make_shared<std::string>(local_base_path));
  }

  // TODO move threading logic here
  bool currently_stopping {false};
  while(current_num < max_num_images)
  {
    if(this->stop_video_saving and !currently_stopping) //flag set in dispense_manager to indicate time to stop generating new frames
    {
      int delay = this->stop_video_delay; //this delay is in ms
      int remaining_frames = std::ceil(delay * fps / 1000);
      Logger::Instance()->Info("Stop Video signal received by generator, with delay: {} ms, equivalent to {} frames", delay, remaining_frames);
      max_num_images = remaining_frames + current_num;
      currently_stopping = true;
    }
    auto start = std::chrono::high_resolution_clock::now();
    std::chrono::system_clock::time_point timePt = start + std::chrono::milliseconds(delay_time);

    char file_name_cstr[24];
    sprintf(file_name_cstr, "image%06d.jpg", current_num);
    this->save_frame(session, local_base_path, std::string(file_name_cstr));

    std::this_thread::sleep_until(timePt);
    auto end = std::chrono::high_resolution_clock::now();
    current_num += 1;
  }
  if (send_mp4) {
      std::string video_path_name = FileSystemUtil::split_basedir(local_base_path).second;
      video_path_name= video_path_name.append(".mp4");
      int video_gen_exit = this->generate_video(local_base_path, video_path_name, "image%06d.jpg", fps);
      if (video_gen_exit == 0) {
          this->send_it(
            sender, local_base_path, FileSystemUtil::split_basedir(remote_base_path).first, video_path_name, true);
      }
  }

  Logger::Instance()->Debug("Video Generator Upload Complete for directory: {}", local_base_path);
}

void UploadGenerator::send_it(std::shared_ptr<FileSender> sender, std::string local_file_path, std::string remote_file_path, std::string file_name, bool delete_post_upload)
{

  FileSystemUtil::join_append(local_file_path, file_name);
  FileSystemUtil::join_append(remote_file_path, file_name);
  sender->send_file(local_file_path, remote_file_path, delete_post_upload,
                    false);

}

void fulfil::depthcam::data::UploadGenerator::signal_stop(int stop_video_delay_sec) {
  stop_video_delay = stop_video_delay_sec;
  stop_video_saving = true;
  usleep(250000); //sleep for long enough for stop flag to take effect, even with framerate pause in save_many_frames

}
void fulfil::depthcam::data::UploadGenerator::signal_stop() {
  stop_video_delay = 0;
  stop_video_saving = true;
}
