//
// Created by Amber Thomas on 4/16/22.
//

#ifndef FULFIL_DISPENSE_TEST_TRAYMANAGER_H
#define FULFIL_DISPENSE_TEST_TRAYMANAGER_H
#include "Fulfil.Dispense/visualization/make_media.h"
#include <Fulfil.CPPUtils/httplib.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.DepthCam/core/session.h>
#include <Fulfil.DepthCam/data/data_generator.h>
#include <Fulfil.DepthCam/data/gcs_sender.h>
#include <Fulfil.Dispense/recipes/tray_calibration_manager.h>
#include <Fulfil.Dispense/recipes/tray_dimensional_configuration.h>
#include <Fulfil.Dispense/recipes/tray_item_distance_calculation_settings.h>

namespace fulfil::dispense::tray
{
  class TrayManager
  {
  private:
    std::shared_ptr<fulfil::depthcam::Session> session;
    fulfil::configuration::tray::TrayDimensions tray_builder;

  public:
    TrayManager(
        const std::shared_ptr<fulfil::depthcam::Session> &session,
        const configuration::tray::TrayDimensions &trayBuilder);

    template <typename ParsedRequest>
    std::experimental::filesystem::path make_default_datagen_path(const std::string &basedir,
                                                                  const ParsedRequest &parsed_object)
    {
      std::experimental::filesystem::path tray_data_fs_path = make_media::paths::add_basedir_date_suffix_and_join(
          basedir, "Tray_Camera/");
      tray_data_fs_path /= parsed_object.get_primary_key_id();
      return tray_data_fs_path;
    }

    // Should possibly generalize a free later
    fulfil::depthcam::data::DataGenerator
    build_tray_data_generator(const std::shared_ptr<nlohmann::json> &original_request,
                              const std::experimental::filesystem::path &tray_datagen_path)
    {
      return fulfil::depthcam::data::DataGenerator(this->session, std::make_unique<std::string>(tray_datagen_path.string()), original_request);
    }

    Tray create_tray(dimensional_info::TrayRecipe tray_recipe);

    int upload_tray_data(const comms_context::RequestContextInfo &request_context,
                         std::string local_root_path,
                         depthcam::data::GCSSender tray_gcs_sender);

    bool save_tray_audit_image(const comms_context::RequestContextInfo &request_context,
                               std::string local_root_path,
                               double scale_resize, 
                               std::shared_ptr<cv::RotateFlags> rotate_code);

    // TODO make a std::optional? Also almost certain I can move the
    //  center generation out of algo
    httplib::Result query_item_counts(
        const request_from_vlsg::TrayRequest &tray_request_obj,
        const std::vector<tray_count_api_comms::LaneCenterLine> &transformed_lane_center_pixels,
        const std::string &image_to_count);

    results_to_vlsg::TrayValidationCounts dispatch_request_to_count_api(
        const request_from_vlsg::TrayRequest &tray_request_obj,
        const std::vector<tray_count_api_comms::LaneCenterLine> &transformed_lane_center_pixels,
        const std::string &saved_images_base_directory);

    // set image rotation angle to compensate for rotated cameras
    void set_image_rotation_angle(float angle);
  };

}

#endif // FULFIL_DISPENSE_TEST_TRAYMANAGER_H
