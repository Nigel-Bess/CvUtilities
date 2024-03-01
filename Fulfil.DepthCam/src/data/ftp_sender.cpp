//
// Created by Amber Thomas on 7/29/21.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//



#include <Fulfil.CPPUtils/curl_control.h>
#include "Fulfil.DepthCam/data/ftp_sender.h"
using fulfil::depthcam::data::FTPSender;
using fulfil::utils::CurlController;

FTPSender::FTPSender(const std::string &remote_root_dir)
{
  this->local_root_dir = "";
  this->remote_root_dir = remote_root_dir;
}

FTPSender::FTPSender(const std::string &local_root_dir, const std::string &remote_root_dir)
{
  this->local_root_dir = local_root_dir;
  this->remote_root_dir = remote_root_dir;
}

int FTPSender::send_file(const std::string &local_path,
                         const std::string &remote_path, bool delete_files,
                         bool use_local_basename) {

  CurlController curly = CurlController(local_root_dir, remote_root_dir);
  return curly.send_file(local_path, remote_path, delete_files);
}



