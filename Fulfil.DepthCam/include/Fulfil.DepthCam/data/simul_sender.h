//
// Created by Amber Thomas on 7/29/21.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DEPTHCAM_SIMUL_SENDER_H
#define FULFIL_DEPTHCAM_SIMUL_SENDER_H

#include <memory>
#include <string>
#include <vector>
#include <Fulfil.DepthCam/data/file_sender.h>
#include <Fulfil.DepthCam/data/gcs_sender.h>
#include <Fulfil.DepthCam/data/ftp_sender.h>
#include <Fulfil.CPPUtils/curl_control.h>

/**
 * FTP_GCS_VideoSender uploads single files to both an FTP and GCS location defined
 * in constructor. Since hitting a GCS endpoint for individual files
 * Can be passed to a generator to handle sending the newly generated files.
 */

namespace fulfil
{
    namespace depthcam
    {
        namespace data
        {
            class SimulSender : public FileSender
            {
            private:
                // Todo make more general with a vector of sender once GCP syncing better
                std::shared_ptr<FileSender> ftp_sender;
                std::shared_ptr<GCSSender> gcs_sender;

            public:
                //"ftp://depthcam:pw4depthcam@192.168.168.5/RawCameraData/<commandId>/"
                //factory-media/<storeId>/dispenses/<commandId>
                //factory-media/<storeId>/trays/<trayId>/<commandId>.jpg
                SimulSender(const std::string& local_root_dir, const std::string& ftp_root_dir,
                                    const std::string& gcs_bucket_name, const std::string& store_id);

                SimulSender(std::shared_ptr<FileSender> ftp_sender, std::shared_ptr<GCSSender> gcs_sender);
                int send_file(const std::string &local_path,
                              const std::string &remote_path, bool delete_files,
                              bool use_local_basename) override;

            };
        }  // namespace mocks
    }  // namespace core
}  // namespace fulfil

#endif