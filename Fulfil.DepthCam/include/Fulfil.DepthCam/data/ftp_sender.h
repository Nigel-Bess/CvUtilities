//
// Created by Amber Thomas on 7/29/21.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * FTPSender uploads single files to FTP location defined in constructor.
 * Can be passed to a generator to handle sending the newly generated files.
 */
#ifndef FULFIL_DEPTHCAM_FTP_SENDER_H
#define FULFIL_DEPTHCAM_FTP_SENDER_H

#include <memory>
#include <string>
#include <vector>
#include <Fulfil.DepthCam/data/file_sender.h>
#include <Fulfil.CPPUtils/curl_control.h>



namespace fulfil
{
    namespace depthcam
    {
        namespace data
        {
            class FTPSender : public fulfil::depthcam::data::FileSender
            {
            private:


                std::string local_root_dir;
                std::string remote_root_dir;

            public:
                //"ftp://depthcam:pw4depthcam@192.168.168.5/"
                explicit FTPSender(const std::string &remote_root_dir);
                FTPSender(const std::string &local_root_dir, const std::string &remote_root_dir);
                int send_file(const std::string &local_path,
                              const std::string &remote_path, bool delete_files,
                              bool use_local_basename=false) override;

            };
        }  // namespace mocks
    }  // namespace core
}  // namespace fulfil

#endif