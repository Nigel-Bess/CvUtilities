//
// Created by Amber Thomas on 7/29/21.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//
#include <Fulfil.CPPUtils/file_system_util.h>
#include "Fulfil.DepthCam/data/ftp_gcs_videosender.h"
#include <Fulfil.CPPUtils/logging.h>
using fulfil::depthcam::data::FTP_GCS_VideoSender;
using fulfil::utils::FileSystemUtil;
using fulfil::utils::Logger;




// TODO should probably just generalize to a list, but need to think about guarding against deleting early
// A GCS sender needs to come last
FTP_GCS_VideoSender::FTP_GCS_VideoSender(const std::string& local_root_dir, const std::string& ftp_root_dir,
                                       const std::string& gcs_bucket_name, const std::string& store_id)
{
    this->ftp_sender = std::make_shared<FTPSender>(local_root_dir, ftp_root_dir);
    this->gcs_sender = std::make_shared<GCSSender>(gcs_bucket_name, store_id);

}

FTP_GCS_VideoSender::FTP_GCS_VideoSender(std::shared_ptr<FTPSender> ftp_sender, std::shared_ptr<GCSSender> gcs_sender)
{
    this->ftp_sender = ftp_sender;
    this->gcs_sender = gcs_sender;
}

int FTP_GCS_VideoSender::send_file(const std::string &local_path,
                                   const std::string &remote_path,
                                   bool delete_files, bool use_local_basename) {
  std::string ext = FileSystemUtil::get_extension(local_path);
  if (std::string(".mp4").compare(ext) != 0) {
    this->ftp_sender->send_file(local_path, remote_path, delete_files, false);
    return 0;
  }
  this->gcs_sender->send_file(local_path, remote_path, delete_files, false);
  if (delete_files) {
    std::string to_delete = FileSystemUtil::split_basedir(local_path).first;
    FileSystemUtil::join_append(to_delete, "*.jpg");
    //std::cout << "Delete_path " << to_delete << std::endl;
    Logger::Instance()->Info("Deleting files {}", to_delete);
    FileSystemUtil::remove_file(to_delete);
  }
  return 0;
}


