//
// Created by Amber Thomas on 7/29/21.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * The purpose of this file is to outline the functionality to save
 * data to ftp server (videos or files from folder)
 */
#ifndef FULFIL_DEPTHCAM_GCS_SENDER_H
#define FULFIL_DEPTHCAM_GCS_SENDER_H

#include <memory>
#include <string>
#include <vector>
#include <Fulfil.DepthCam/data/file_sender.h>
#include <Fulfil.CPPUtils/httplib.h>

namespace fulfil::depthcam::data
{
    class GCSSender final : public fulfil::depthcam::data::FileSender
    {
    private:
        std::string cloud_root_dir;
        //Maybe pre made version of the json?
        std::string make_json_request(const std::string &local_path,
                                      const std::string &remote_path,
                                      bool delete_files,
                                      bool use_local_basename);
    public:
        GCSSender(const std::string& bucket_name, const std::string& store_id);
        explicit GCSSender(const std::string& cloud_root_dir);
        int send_file(const std::string &local_path,
                      const std::string &remote_path, bool delete_files,
                      bool use_local_basename) override;

        // Maybe some kind of done with folder signal would be good
    };
}  // namespace fulfil

#endif