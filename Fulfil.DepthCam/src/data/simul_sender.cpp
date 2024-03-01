//
// Created by Amber Thomas on 7/29/21.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//
#include "Fulfil.DepthCam/data/simul_sender.h"
using fulfil::depthcam::data::SimulSender;




// TODO should probably just generalize to a list, but need to think about guarding against deleting early
// A GCS sender needs to come last
SimulSender::SimulSender(const std::string& local_root_dir, const std::string& ftp_root_dir,
                                       const std::string& gcs_bucket_name, const std::string& store_id)
{
    this->ftp_sender = std::make_shared<FTPSender>(local_root_dir, ftp_root_dir);
    this->gcs_sender = std::make_shared<GCSSender>(gcs_bucket_name, store_id);

}

SimulSender::SimulSender(std::shared_ptr<FileSender> ftp_sender, std::shared_ptr<GCSSender> gcs_sender)
{
    this->ftp_sender = ftp_sender;
    this->gcs_sender = gcs_sender;
}

int SimulSender::send_file(const std::string &local_path,
                           const std::string &remote_path, bool delete_files,
                           bool use_local_basename) {
  this->ftp_sender->send_file(local_path, remote_path, false, false);
  this->gcs_sender->send_file(local_path, remote_path, delete_files, false);
  return 0;
}


