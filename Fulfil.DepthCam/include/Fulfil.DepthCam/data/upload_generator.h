//
// Created by Amber Thomas on 7/29/21.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * The purpose of this file is to outline the functionality to save
 * data to ftp server (videos or files from folder)
 */
#ifndef FULFIL_DEPTHCAM_UPLOAD_GENERATOR_H
#define FULFIL_DEPTHCAM_UPLOAD_GENERATOR_H

#include <memory>
#include <string>
#include <atomic>

#include <Fulfil.DepthCam/core/session.h>
#include <Fulfil.DepthCam/data/file_sender.h>



namespace fulfil::depthcam::data {
// TODO Rename to video generator
class UploadGenerator
{
    private:
        // object that handles method and location of file transfers
        //std::shared_ptr<fulfil::depthcam::data::FileSender> sender;


        void send_it(std::shared_ptr<FileSender> sender, std::string local_file_path, std::string remote_file_path, std::string file_name, bool delete_post_upload);

        int generate_video(std::string video_file_path, std::string video_file_name, std::string name_fmt, int fps);

 public:

    UploadGenerator() = default;
    explicit UploadGenerator(int default_stop_video_delay);

  void save_many_frames(std::shared_ptr<fulfil::depthcam::Session> session, std::shared_ptr<FileSender> sender, int duration, int fps,
                        std::string local_base_path, std::string remote_base_path, bool send_frames, bool send_mp4);

  void save_frame(std::shared_ptr<fulfil::depthcam::Session> session, std::string image_path,
          std::string image_name, float resize_factor=0.29);

  void signal_stop(int stop_video_delay_sec);
  void signal_stop();
  std::atomic_bool stop_video_saving {true};
  int stop_video_delay {0}; // atomic?



};
}


#endif