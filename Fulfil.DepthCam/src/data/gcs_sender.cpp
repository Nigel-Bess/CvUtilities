//
// Created by Amber Thomas on 7/29/21.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//

#include <Fulfil.CPPUtils/httplib.h>
#include <Fulfil.CPPUtils/logging.h>
#include "Fulfil.DepthCam/data/gcs_sender.h"
#include <experimental/filesystem>
#include <Fulfil.CPPUtils/timer.h>


namespace std_filesystem = std::experimental::filesystem;
using fulfil::depthcam::data::GCSSender;
using fulfil::utils::Logger;


// For abis full path should be factory-media/<storeId>/dispenses/<dispenseId>/
// For tray audit full path is factory-media/<storeId>/tray_audits/<tray_id>/<dispenseId>.jpg
GCSSender::GCSSender(const std::string& bucket_name, const std::string& store_id)
{
    std_filesystem::path cloud_path{bucket_name};
    cloud_path /= std_filesystem::path {store_id};
    this->cloud_root_dir = cloud_path.string();
}

GCSSender::GCSSender(const std::string& cloud_root_dir)
{
    this->cloud_root_dir = cloud_root_dir;
}

std::string GCSSender::make_json_request(const std::string &local_path,
  const std::string &remote_path,
  bool delete_files,
  bool use_local_basename) {
    std::string del_files_str = delete_files ? "true" : "false";
    std::string use_local_basename_str = use_local_basename ? "true" : "false";
    std::string req = R"({"Cloud_Base_Url":")" + this->cloud_root_dir + R"(","Cloud_Data_Path":")" + remote_path
                      + R"(","Device_Data_Path":")" + local_path + R"(","delete_post_upload":)" + del_files_str
                      + R"(,"use_local_basename":)" + use_local_basename_str + "}";
    Logger::Instance()->Trace("Created GCS upload request.");
    return req;
}

int GCSSender::send_file(const std::string &local_path,
  const std::string &remote_path, bool delete_files,
  bool use_local_basename) {
    auto timer = fulfil::utils::timing::Timer("GCSSender::send_file " + local_path);

    if (!std_filesystem::exists(local_path)) {
        fulfil::utils::Logger::Instance()->Error("Attempted to upload a non existent file {}", local_path);
        return 0;
    }

    std::string req_body =
      make_json_request(local_path, remote_path, delete_files, use_local_basename);
    Logger::Instance()->Debug("About to post request");
    httplib::Client cli("localhost", 5002);
    if (auto res = cli.Post("/inference/uploadfile", httplib::Headers(), req_body, "application/json")) {
        Logger::Instance()->Debug("Return status from upload enqueue call: {} , {}", res->status, res->body);
        return 1;
    }
    Logger::Instance()->Error("Return error from uploader! Unable to connect to upload queue!"
      "\nError GCS upload request:\n\t{}", req_body);
    return 0;
}



