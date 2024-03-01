//
// Created by vincent on 09/05/22.
//
#include "Fulfil.DepthCam/data/bigquery_upload.h"
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/httplib.h>

using fulfil::depthcam::data::BQUpload;
using fulfil::utils::Logger;

int BQUpload::upload_traycount_record(nlohmann::json validation_request, nlohmann::json validation_response,
                                      std::string vls_name, std::string calibration){

    BQUpload uploader;

    nlohmann::json bq_doc = validation_request;
    bq_doc["CalibrationId"] = calibration;
    bq_doc["vls_name"] = vls_name;
    bq_doc.update(validation_response, true);

    return uploader.upload_record("fulfil-web.factory0.TrayCounts", bq_doc);
}

int BQUpload::upload_record(const std::string table_name, nlohmann::json data){
    nlohmann::json request = data;
    request["table_id"] = table_name;
    std::string str_request = request.dump();
    Logger::Instance()->Debug("Request sent to Bigquery uploader is: \n\t{}.", str_request);
    httplib::Client cli("localhost", 5002);
    if (auto res = cli.Post("/inference/bq_upload", httplib::Headers(), str_request, "application/json")) {
        Logger::Instance()->Debug("Return status from BQ upload queue: {}.", res->status);
        return 0;
    } else {
        Logger::Instance()->Error("Return error from uploader! Unable to connect to upload queue!"
                                  "\nError BQ upload request:\n\t{}", str_request);
        return -1;
    }
}
