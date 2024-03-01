//
// Created by steve on 2/25/20.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * The purpose of this file is to outline the functionality to save
 * data to ftp server (videos or files from folder)
 */
#ifndef FULFIL_DEPTHCAM_FTP_GENERATOR_H
#define FULFIL_DEPTHCAM_FTP_GENERATOR_H

#include <memory>
#include <string>

#include <Fulfil.DepthCam/core/session.h>

namespace fulfil
{
namespace depthcam
{
namespace data
{
class FTPGenerator
{
 private:
  //base FTP directory to be used in CURL commands, e.g. "ftp://depthcam:pw4depthcam@192.168.168.5/"
  std::string FTP_base;

  std::shared_ptr<bool> stop_video_saving;

 public:
  /**
   * Constructor
   * @param session the session which will be used to generate the data to save.
   */
  FTPGenerator(std::string ftp_base, std::shared_ptr<bool> stop_video_saving);

  /**
   *
   * Saves reduces size frames to local disk for duration seconds at frequency fps, from the provided camera in session
   *
   */
  void save_many_frames(std::shared_ptr<fulfil::depthcam::Session> session, int duration, int fps,
                        std::string local_base_path, std::string ftp_path_appendix);

  void save_frame(std::shared_ptr<fulfil::depthcam::Session> session, std::string image_path,
          std::string image_name, float resize_factor=0.25);

  void upload_files_from_folder(std::vector<std::string> file_names, std::string local_base_path, std::string FTP_path_appendix);
};
}  // namespace mocks
}  // namespace core
}  // namespace fulfil


#endif