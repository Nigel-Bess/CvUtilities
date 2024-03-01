//
// Created by Amber Thomas on 7/12/23.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * A sender that does not send
 */
#ifndef FULFIL_DEPTHCAM_NULL_SENDER_H
#define FULFIL_DEPTHCAM_NULL_SENDER_H

#include <string>
#include <Fulfil.DepthCam/data/file_sender.h>

namespace fulfil::depthcam::data
{
  class NullSender  : public fulfil::depthcam::data::FileSender
  {
      // Null class to input to not send data
    public:
      NullSender()=default;
      int send_file(const std::string &local_path,
        const std::string &remote_path,
        bool delete_files,
        bool use_local_basename);
  };
  inline int NullSender::send_file(const std::string &local_path, const std::string &remote_path,
    bool delete_files, bool use_local_basename)
  { return 0 ; }
}

#endif