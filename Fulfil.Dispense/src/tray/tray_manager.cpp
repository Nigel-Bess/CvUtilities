//
// Created by Amber Thomas on 4/16/22.
//
#include <Fulfil.Dispense/tray/tray_manager.h>
#include <Fulfil.CPPUtils/file_system_util.h>
#include <Fulfil.Dispense/visualization/make_media.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/timer.h>
#include <Fulfil.DepthCam/data/gcs_sender.h>
#include <utility>

using fulfil::dispense::tray::Tray;
using fulfil::dispense::tray::TrayManager;
namespace std_filesystem = std::experimental::filesystem;


TrayManager::TrayManager(
    const std::shared_ptr<fulfil::depthcam::Session>& session,
    const fulfil::configuration::tray::TrayDimensions &trayBuilder)
    : session(session), tray_builder(trayBuilder) {}

#pragma region data_saving
std_filesystem::path make_tray_audit_image_basename(const comms_context::RequestContextInfo &request_context){
    return make_media::paths::join_as_path("trays", request_context.m_tray_id.m_val, request_context.get_primary_key_id() + ".jpg");
}

bool TrayManager::save_tray_audit_image(
  const comms_context::RequestContextInfo &request_context, std::string local_root_path, double scale_resize, std::shared_ptr<cv::RotateFlags> rotate_code)
{
    std_filesystem::path const path_suffix {make_tray_audit_image_basename(request_context)};
    std_filesystem::path const tray_audit_local_file { local_root_path / path_suffix } ;

    try {
        auto timer = fulfil::utils::timing::Timer("TrayManager::save_tray_audit_image " + path_suffix.string() );
        cv::Mat color_mat = this->session->grab_color_frame();
        
        // rotate the image if needed
        if (rotate_code != nullptr)
        {
            cv::Mat rotated_image;
            cv::rotate(color_mat, rotated_image, *rotate_code);
            color_mat = rotated_image.clone();
        }
        if (scale_resize < 1 ){ cv::resize(color_mat, color_mat, cv::Size(), scale_resize, scale_resize); }
        std_filesystem::create_directories(tray_audit_local_file.parent_path());
        fulfil::utils::Logger::Instance()->Debug("Saving tray audit image to {}", tray_audit_local_file.string());
        return cv::imwrite(tray_audit_local_file.string(), color_mat);
    } catch(const std::exception & e) {
        fulfil::utils::Logger::Instance()->Error("Issue saving tray audit data for {}: \n\t{}",
          path_suffix.string(), e.what());
    }
    return false;
}

// TODO the intention is to wrap this as a lambda, make free or include store_id
//  in state? Considering a context params struct with path info
int TrayManager::upload_tray_data(const comms_context::RequestContextInfo &request_context,
                                                          std::string local_root_path,
                                                          depthcam::data::GCSSender tray_gcs_sender) {
    std_filesystem::path path_suffix {make_tray_audit_image_basename(request_context)};
    std_filesystem::path tray_audit_local_file { local_root_path / path_suffix } ;
    // it looks like there should have been a save data function here.
    // TODO make a data generator, and possibly FED visualization here
    fulfil::utils::Logger::Instance()->Debug("Sending file {}", tray_audit_local_file.string());
    return tray_gcs_sender.send_file(tray_audit_local_file.string(),
                                path_suffix.string(), true, false);

}

#pragma endregion data_saving

httplib::Result TrayManager::query_item_counts(
    const request_from_vlsg::TrayRequest &tray_request_obj,
    const std::vector<tray_count_api_comms::LaneCenterLine> &transformed_lane_center_pixels,
    const std::string &image_to_count) {
    // Basic http params
   auto timer = fulfil::utils::timing::Timer("TrayManager::query_item_counts for " + tray_request_obj.m_context.get_id_tagged_sequence_step() );
    constexpr int tray_count_port = 5002;
    httplib::Client cli("localhost", tray_count_port);
    cli.set_read_timeout(1, 500000); // 0.5 second timeout
    // Complete generation of count body
    auto to_count_api_request = nlohmann::json (tray_request_obj);
    to_count_api_request["Centers"] = transformed_lane_center_pixels;
    to_count_api_request["ImagePath"] = image_to_count;
  return cli.Post("/inference/locatetrayitems", httplib::Headers(), to_count_api_request.dump(), "application/json");
}

Tray TrayManager::create_tray(
    dimensional_info::TrayRecipe tray_recipe) {
    return this->tray_builder.build_tray_from_recipe(std::move(tray_recipe));
}

results_to_vlsg::TrayValidationCounts TrayManager::dispatch_request_to_count_api(
  const request_from_vlsg::TrayRequest &tray_request_obj,
  const std::vector<tray_count_api_comms::LaneCenterLine> &transformed_lane_center_pixels,
  const std::string &saved_images_base_directory)
{
    auto image_to_count =  make_media::paths::join_as_path(this->make_default_datagen_path(
                    saved_images_base_directory, tray_request_obj), tray_request_obj.get_sequence_step(), "color_image.png");
    fulfil::utils::Logger::Instance()->Debug("Params sent to TrayCountAPI: request is {}, transformed lane center pixels are, image path is {}", tray_request_obj.to_string(), std::string(image_to_count));
    auto count_res = this->query_item_counts(tray_request_obj, transformed_lane_center_pixels, image_to_count);
    if (bool(count_res) && count_res->status == 200) {
        fulfil::utils::Logger::Instance()->Info("Return status from count algo: {}.", count_res->status);
        fulfil::utils::Logger::Instance()->Debug("TrayCountAPI response body before parsing: {}", count_res->body);
        nlohmann::json doc = nlohmann::json::parse(count_res->body);
        doc["status"] = count_res->status;
        return doc["count_results"].get<results_to_vlsg::TrayValidationCounts>();
    }
    fulfil::utils::Logger::Instance()->Error("Tray count algo did not return in Single Lane Request! API is likely down!");
    nlohmann::json doc = {{ "status", 500 }, {"Errors", {2} } };
    doc.update(tray_request_obj.m_context);
    return doc.get<results_to_vlsg::TrayValidationCounts>();

}
