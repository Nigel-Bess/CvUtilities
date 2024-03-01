//
// Created by Amber Thomas on 7/29/21.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * The purpose of this file is to outline the functionality to save
 * data to ftp server (videos or files from folder)
 */
#ifndef FULFIL_DEPTHCAM_FILE_SENDER_H
#define FULFIL_DEPTHCAM_FILE_SENDER_H

#include <memory>
#include <string>

namespace fulfil::depthcam::data
{
    class FileSender
    {
    // Abstract class for a ftp or cloud send call to allow for quick integration into code base
    // That being said, once the live viewer is depreciated, I think we should overhaul
    public:
        virtual int send_file(const std::string &local_path,
                            const std::string &remote_path,
                            bool delete_files,
                            bool use_local_basename) = 0;

    };
}

#endif