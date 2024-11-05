#include <experimental/filesystem>
#include <memory>

#include <Fulfil.CPPUtils/file_system_util.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/point_3d.h>
#include <Fulfil.CPPUtils/timer.h>
#include <Fulfil.DepthCam/aruco.h>
#include <Fulfil.DepthCam/core.h>
#include <Fulfil.DepthCam/point_cloud.h>
#include "Fulfil.Dispense/dispense/dispense_manager.h"
#include "Fulfil.Dispense/dispense/drop_error_codes.h"
#include "Fulfil.Dispense/dispense/side_dispense_error_codes.h"
#include <Fulfil.Dispense/drop/drop_grid.h>
#include "Fulfil.Dispense/drop/drop_manager.h"
#include <Fulfil.Dispense/drop/drop_result.h>
#include <Fulfil.Dispense/drop/drop_zone_searcher.h>
#include <Fulfil.Dispense/drop/pre_post_compare.h>
#include <Fulfil.Dispense/drop/drop_result.h>
#include <Fulfil.Dispense/drop/side_drop_result.h>
#include <Fulfil.Dispense/visualization/make_media.h>
#include <Fulfil.Dispense/visualization/live_viewer.h>

namespace std_filesystem = std::experimental::filesystem;

using fulfil::depthcam::DepthSession;
using fulfil::configuration::lfb::LfbVisionConfiguration;
using fulfil::depthcam::aruco::Container;
using fulfil::depthcam::aruco::MarkerDetector;
using fulfil::depthcam::aruco::MarkerDetectorContainer;
using fulfil::depthcam::data::DataGenerator;
using fulfil::dispense::commands::PostLFRResponse;
using fulfil::dispense::commands::PreSideDispenseResponse;
using fulfil::dispense::drop::DropGrid;
using fulfil::dispense::drop::DropManager;
using fulfil::dispense::drop::DropResult;
using fulfil::dispense::drop::DropZoneSearcher;
using fulfil::dispense::drop::pre_post_compare_error_codes::PrePostCompareErrorCodes;
using fulfil::dispense::drop::SideDropResult;
using fulfil::dispense::drop_target_error_codes::DropTargetErrorCodes;
using fulfil::depthcam::pointcloud::LocalPointCloud;
using fulfil::dispense::side_dispense_error_codes::SideDispenseErrorCodes;
using fulfil::dispense::side_dispense_error_codes::SideDispenseError;
using fulfil::dispense::visualization::LiveViewer;
using fulfil::dispense::visualization::ViewerImageType;
using fulfil::utils::FileSystemUtil;
using fulfil::utils::Logger;
using fulfil::utils::Point3D;

DropManager::DropManager(std::shared_ptr<fulfil::depthcam::Session> session, std::shared_ptr<INIReader> dispense_man_reader,
                         std::shared_ptr<fulfil::dispense::visualization::LiveViewer> drop_live_viewer)
{
  this->session = session;
  this->drop_live_viewer = drop_live_viewer;

  int debug = dispense_man_reader->GetInteger("drop_zone_searcher", "debug", -1);
  int visualize = dispense_man_reader->GetInteger("drop_zone_searcher", "visualize", -1);
  int force_error = dispense_man_reader->GetInteger("drop_zone_searcher", "force_error", -1);

  // Parameters for drop zone searcher algorithms
  float shadow_buffer = dispense_man_reader->GetFloat("drop_zone_searcher", "shadow_buffer", -1);
  int item_mass_threshold = dispense_man_reader->GetInteger("drop_zone_searcher", "item_mass_threshold", 500);
  int min_bag_filtering_threshold = dispense_man_reader->GetInteger("drop_zone_searcher", "min_bag_filtering_threshold", -1);
  float white_region_depth_adjust_from_min = dispense_man_reader->GetFloat("drop_zone_searcher", "white_region_depth_adjust_from_min", -1);
  int empty_bag_threshold = dispense_man_reader->GetInteger("drop_zone_searcher", "empty_bag_threshold", -1);

  float interference_depth_tolerance = dispense_man_reader->GetFloat("drop_zone_searcher", "interference_depth_tolerance", -1);
  int num_interference_points_tolerance = dispense_man_reader->GetInteger("drop_zone_searcher", "num_interference_points_tolerance", -1);
  float interference_region_length_factor = dispense_man_reader->GetFloat("drop_zone_searcher", "interference_region_length_factor", -1);
  bool visualize_interference_zone = dispense_man_reader->GetBoolean("drop_zone_searcher", "visualize_interference_zone", true);
  this->acceptable_Z_above_marker_surface = dispense_man_reader->GetFloat("drop_zone_searcher", "acceptable_Z_above_marker_surface", 0);

  float significant_variance_regression = dispense_man_reader->GetFloat("drop_zone_searcher", "significant_variance_regression", -1);
  float moderate_variance_regression = dispense_man_reader->GetFloat("drop_zone_searcher", "moderate_variance_regression", -1);
  float moderate_variance_improvement = dispense_man_reader->GetFloat("drop_zone_searcher", "moderate_variance_improvement", -1);
  float significant_variance_improvement = dispense_man_reader->GetFloat("drop_zone_searcher", "significant_variance_improvement", -1);
  float significant_depth_regression = dispense_man_reader->GetFloat("drop_zone_searcher", "significant_depth_regression", -1);
  float equivalent_depth = dispense_man_reader->GetFloat("drop_zone_searcher", "equivalent_depth", -1);
  float moderate_depth_improvement = dispense_man_reader->GetFloat("drop_zone_searcher", "moderate_depth_improvement", -1);
  float significant_depth_improvement = dispense_man_reader->GetFloat("drop_zone_searcher", "significant_depth_improvement", -1);
  float crazy_depth_regression = dispense_man_reader->GetFloat("drop_zone_searcher", "crazy_depth_regression", -1);
  float crazy_depth_improvement = dispense_man_reader->GetFloat("drop_zone_searcher", "crazy_depth_improvement", -1);

  this->searcher = std::make_shared<DropZoneSearcher>(this->session, visualize, force_error,
                                                      shadow_buffer, item_mass_threshold, drop_live_viewer, debug,
                                                      min_bag_filtering_threshold, white_region_depth_adjust_from_min, empty_bag_threshold,
                                                      interference_depth_tolerance, num_interference_points_tolerance,
                                                      interference_region_length_factor, visualize_interference_zone, acceptable_Z_above_marker_surface,
                                                      significant_variance_regression,
                                                      moderate_variance_regression, moderate_variance_improvement, significant_variance_improvement,
                                                      significant_depth_regression,  equivalent_depth, moderate_depth_improvement,
                                                      significant_depth_improvement, crazy_depth_regression, crazy_depth_improvement);

  this->pre_post_compare = std::make_shared<PrePostCompare>(visualize, this->searcher);
  this->cached_pre_container = nullptr;
  this->cached_post_container = nullptr;
  this->cached_pre_request = nullptr;
  this->cached_post_request = nullptr;
  this->cached_drop_target = nullptr;
  this->cached_drop_damage_code = nullptr;
  this->mongo_bag_state = std::make_shared<fulfil::mongo::MongoBagState>();
}

void DropManager::generate_request_data(bool generate_data,
                                             std_filesystem::path base_directory,
                                             const std::shared_ptr<std::string> &time_stamp,
                                             std::shared_ptr<nlohmann::json> request_json)
{
    // data generation is gated so that offline simulation does not generate data
    if (generate_data)
    {
        Logger::Instance()->Trace("DropManager: generate_data boolean is TRUE, about to generate data in {}", base_directory.string());
        DataGenerator generator = DataGenerator(this->session,
                                                std::make_shared<std::string>(base_directory.string()),
                                                request_json);
        generator.save_data(time_stamp);
    }
}

void DropManager::generate_request_handling_data(bool generate_data,
                                                std_filesystem::path base_directory,
                                                const std::shared_ptr<std::string> &time_stamp,
                                                std::shared_ptr<nlohmann::json> request_json,
                                                std::shared_ptr<nlohmann::json> bag_state_json)
{
    generate_request_data(generate_data, base_directory, time_stamp, request_json);
    // data generation is gated so that offline simulation does not generate data
    if (generate_data)
    {
        if (bag_state_json->is_null())
        {
            Logger::Instance()->Debug("No bag state JSON is available, json file will not be saved along with data generation");
        } else {
            std::string bag_state_file_path = make_media::paths::join_as_path(base_directory, *time_stamp, "bag_state.json");
            Logger::Instance()->Trace("Bag state JSON data generation file path: {}", bag_state_file_path);

            std::ofstream file(bag_state_file_path);
            file << *bag_state_json;
            Logger::Instance()->Trace("Finished bag state JSON data generation for drop camera request!");
        }
    }
}

void DropManager::generate_error_code_result_data(bool generate_data, std::string error_code_file, int error_code)
{
    // data generation is gated so that offline simulation does not generate data
    if (generate_data) {
        // save error/success code
        Logger::Instance()->Trace("Saving error code file with code {} at path {}", error_code, error_code_file);
        std::ofstream error_file(error_code_file);
        error_file << error_code;
    }
}

void DropManager::generate_drop_target_result_data(bool generate_data, std::string target_file, std::string error_code_file, float rover_position, float dispense_position, int error_code)
{
    // data generation is gated so that offline simulation does not generate data
    if (generate_data)
    {
        generate_error_code_result_data(generate_data, error_code_file, error_code);
        // save X,Y of drop target result
        std::ofstream target_file_stream(target_file);
        target_file_stream << rover_position << "\n";
        target_file_stream << -1 * dispense_position
                           << std::endl; //target is saved in LFB frame, requires the sign flip
        Logger::Instance()->Trace("Finished data generation for drop camera request!");
    }
}

void DropManager::generate_floor_view_result_data(bool generate_data, std::string floor_view_file, std::string error_code_file, bool anomaly_detected, bool item_on_ground, float floor_analysis_confidence_score, int error_code)
{
    // data generation is gated so that offline simulation does not generate data
    if (generate_data)
    {
        generate_error_code_result_data(generate_data, error_code_file, error_code);
        // save results of the floor view analysis
        std::ofstream floor_view_file_stream(floor_view_file);
        floor_view_file_stream << anomaly_detected << "\n";
        floor_view_file_stream << item_on_ground  << "\n";
        floor_view_file_stream << floor_analysis_confidence_score
                               << std::endl;
        Logger::Instance()->Trace("Finished data generation for drop camera floor view request!");
    }
}

void get_min_max(cv::Mat m, std::string debugging_string_desc)
{
    double minVal;
    double maxVal;
    cv::Point minLoc;
    cv::Point maxLoc;
    minMaxLoc( m, &minVal, &maxVal, &minLoc, &maxLoc );
    Logger::Instance()->Debug("{}: Number valid points found in matrix: {}\n\tMin: ({},{})={}\n\tMax: ({},{})={}\n\tMean={}",
                              debugging_string_desc, cv::countNonZero(m), minLoc.x, minLoc.y, minVal, maxLoc.x, maxLoc.y, maxVal, cv::mean(m)[0]);
}

std::string get_eigen_stats(Eigen::Matrix3Xd point_cloud){
    Eigen::Matrix3Xd::Index max_index;
    auto max_res = point_cloud.rowwise().sum().maxCoeff(&max_index);
    Eigen::Matrix3Xd::Index min_index;
    auto min_res = point_cloud.rowwise().sum().minCoeff(&min_index);
    std::stringstream out_msg;
    out_msg << "There are " << (point_cloud.array().row(2) == 0).count() << " points at 0 depth out of " << point_cloud.cols()
            << " total. The cloud has a max point sum of " << max_res << " at point " << max_index
            << " and a min point sum of " << min_res << " at " << min_index
            << ".\nMin value across rows is \n" << point_cloud.rowwise().minCoeff()
            << "\nMean value across rows is \n" << point_cloud.rowwise().mean()
            << "\nMax value across rows is \n" << point_cloud.rowwise().maxCoeff();
    return out_msg.str();
}

std::shared_ptr<DropResult> DropManager::handle_drop_request(std::shared_ptr<nlohmann::json> request_json,
                                 std::shared_ptr<fulfil::dispense::commands::DropTargetDetails> details,
                                 const std::shared_ptr<std::string> &base_directory_input,
                                 const std::shared_ptr<std::string> &time_stamp,
                                 bool generate_data,
                                 bool bot_has_already_rotated)
{
    Logger::Instance()->Debug("Handle_Drop_Request called in drop_manager.cpp");
    std::string PrimaryKeyID = (*request_json)["Primary_Key_ID"].get<std::string>();
    this->searcher->PKID = PrimaryKeyID;
    std::shared_ptr<LfbVisionConfiguration> lfb_vision_config = this->mongo_bag_state->raw_mongo_doc->Config;

    std_filesystem::path base_directory = make_media::paths::join_as_path(
      (base_directory_input) ? *base_directory_input: "","Drop_Camera", PrimaryKeyID, "Drop_Target_Image");
    Logger::Instance()->Debug("Base directory is {}", base_directory.string());

    std::string error_code_file = make_media::paths::join_as_path(base_directory, *time_stamp, "error_code");
    std::string target_file = make_media::paths::join_as_path(base_directory, *time_stamp, "target_center");
    std::cout << error_code_file << std::endl;

    Logger::Instance()->Debug("about to lock session");
    try
    {
        //this->session->set_emitter(true); //turn on emitter for imaging

        this->session->refresh();
        //this->session->set_emitter(false); //turn off emitter after imaging

        generate_request_handling_data(generate_data, base_directory, time_stamp, request_json,  std::make_shared<nlohmann::json>(this->mongo_bag_state->GetStateAsJson()));

        Logger::Instance()->Debug("Getting container for algorithm now");
        std::shared_ptr<MarkerDetectorContainer> container = this->searcher->get_container(lfb_vision_config, this->session, lfb_vision_config->extend_depth_analysis_over_markers);

        this->cached_drop_target_container = container; //cache for potential use in prepostcomparison later
        this->cached_drop_target_request = request_json; //cache for potential use in prepostcomparison later
        this->cached_drop_damage_code = std::make_shared<int>(details->item_damage_code);
        this->cached_info = nullptr;

        // Initiate main algorithm to find the target drop center
        std::shared_ptr<DropResult> drop_result = this->searcher->find_drop_zone_center(container, details, lfb_vision_config,
                                                                                        this->mongo_bag_state, bot_has_already_rotated);

        generate_drop_target_result_data(generate_data, target_file, error_code_file, drop_result->rover_position,
                                             drop_result->dispense_position, drop_result->success_code);

        //cache drop target. Target is cached in the LFB local coordinate frame (in meter units), that's why the VLS drop y result is flipped
        this->cached_drop_target = std::make_shared<cv::Point2f>(cv::Point2f(drop_result->rover_position/1000, -1 * drop_result->dispense_position/1000));
        this->cached_info = std::make_shared<std::vector<std::string>>();
        this->cached_info->push_back("Target: Success");
        if(drop_result->LFB_Currently_Rotated) this->cached_info->push_back("LFR is Rotated");
        return drop_result;
    }
    catch (const rs2::unrecoverable_error& e)
    {
        std::string error_descrip = std::string("Realsense Exception: `") + e.what() +
                             std::string("`\nIn function: `") + e.get_failed_function() +
                             std::string("`\nWith args: `") + e.get_failed_args() + std::string("`");
        Logger::Instance()->Fatal(error_descrip);
        return std::make_shared<DropResult>(details->request_id, DropTargetErrorCodes::UnrecoverableRealSenseError, error_descrip);
    }
    catch (const rs2::recoverable_error& e)
    {
        std::string error_msg = std::string("Realsense Exception: `") + e.what() +
                         std::string("`\nIn function: `") + e.get_failed_function() +
                         std::string("`\nWith args: `") + e.get_failed_args() + std::string("`");
        Logger::Instance()->Error(error_msg);

        return std::make_shared<DropResult>(details->request_id, DropTargetErrorCodes::RecoverableRealSenseError, error_msg);
    }
    catch (const std::invalid_argument& e)
    {
        std::string error_description = std::string("Invalid Argument Exception: ") + e.what();
        Logger::Instance()->Error(error_description);
        return std::make_shared<DropResult>(details->request_id, DropTargetErrorCodes::NoMarkersDetected, error_description);
    }
    catch (drop_target_error_codes::DropTargetError & e)
    {
        DropTargetErrorCodes error_id = e.get_status_code();
        Logger::Instance()->Info("DropManager failed handling drop request: {}", e.what());

        // TODO: should pre_target be generated as well? will that overwrite if already written or append
        generate_drop_target_result_data(generate_data, target_file, error_code_file, -1, -1, error_id);
        return std::make_shared<DropResult>(details->request_id, error_id, e.get_description());
    }
    // TODO - remove. this is horrible. sincerely, jess
    catch (std::tuple<int, std::string> & e)
    {
        DropTargetErrorCodes error_id = (DropTargetErrorCodes)std::get<int>(e);
        std::string error_desc = std::get<std::string>(e);
        Logger::Instance()->Info("DropManager failed handling drop request: {}", error_desc);

        generate_drop_target_result_data(generate_data, target_file, error_code_file, -1, -1, error_id);
        return std::make_shared<DropResult>(details->request_id, error_id, error_desc);
    }
    catch (const std::exception & e)
    {
        std::string error_desc = std::string("Unspecified failure from DropManager handling drop request with error:\n{}") + e.what();
        Logger::Instance()->Error(error_desc);
        return std::make_shared<DropResult>(details->request_id, DropTargetErrorCodes::UnspecifiedError,
                                            error_desc);
    }
    catch (...)
    {
      Logger::Instance()->Error("Unspecified failure from DropManager handling drop request");
      return std::make_shared<DropResult>(details->request_id, DropTargetErrorCodes::UnspecifiedError, "In `catch (...)` block in drop_manager");
    }
}


std::shared_ptr<PostLFRResponse> DropManager::handle_post_LFR(std::shared_ptr<nlohmann::json> request_json,
                                                              const std::shared_ptr<std::string> &base_directory_input,
                                                              std::shared_ptr<std::string> request_id,
                                                              bool generate_data)
{
    std::string PrimaryKeyID = (*request_json)["Primary_Key_ID"].get<std::string>();
    std::shared_ptr<LfbVisionConfiguration> lfb_vision_config = this->mongo_bag_state->raw_mongo_doc->Config;
    auto timer = fulfil::utils::timing::Timer("DropManager::handle_post_LFR for " + PrimaryKeyID);
    auto time_stamp = make_media::paths::get_datetime_str();

    std_filesystem::path base_directory = make_media::paths::join_as_path(
        (base_directory_input) ? *base_directory_input: "","Drop_Camera", PrimaryKeyID);
    std::string error_code_file = ("Post_Drop_Image" / base_directory / time_stamp / "error_code").string();
    Logger::Instance()->Debug("Base directory is {}", base_directory.string());

  try
  {
    Logger::Instance()->Trace("Refresh Session Called in Dispense Manager -> Handle Post Dispense");

    this->session->refresh(); //need to refresh the session to get updated frames

    //get image for live viewer
    if(this->drop_live_viewer != nullptr) this->drop_live_viewer->update_image(this->session->get_color_mat(), ViewerImageType::LFB_Post_Dispense, PrimaryKeyID);
 

    std::shared_ptr<DataGenerator> generator;
    std::string data_destination = (base_directory / "Post_Drop_Image").string();

    generate_request_handling_data(generate_data, data_destination, std::make_shared<std::string>(time_stamp), request_json, std::make_shared<nlohmann::json>(this->mongo_bag_state->GetStateAsJson()));
    Logger::Instance()->Debug("Getting container for algorithm now");
    bool extend_depth_analysis_over_markers = lfb_vision_config->extend_depth_analysis_over_markers;
    std::shared_ptr<MarkerDetectorContainer> container = this->searcher->get_container(lfb_vision_config, this->session, lfb_vision_config->extend_depth_analysis_over_markers);


    this->cached_post_container = container; //cache container for potential use in prepostcomparison later
    this->cached_post_request = request_json;

    std::shared_ptr<PostLFRResponse> post_drop_result = this->searcher->find_max_Z(container, request_id, lfb_vision_config,
                                                                                    this->mongo_bag_state, request_json, this->cached_info, std::make_shared<std::string>(base_directory));

    generate_error_code_result_data(generate_data, error_code_file, post_drop_result->get_success_code());


    return post_drop_result;
  }
  catch (int error_id)
  {
    Logger::Instance()->Info("DropManager failed handling post drop request with error id: {}", error_id);
    if (error_id == 1 or error_id == 2)
    {
      if(this->cached_info != nullptr and this->drop_live_viewer != nullptr)
      {
        this->cached_info->push_back("Item Detect Failed");
        this->cached_info->push_back("Not Enough Markers");
        //use cached info from dispense to create a live visualization of the info to be displayed on FDT
        this->drop_live_viewer->update_image(this->drop_live_viewer->get_blank_visualization(), ViewerImageType::Info, PrimaryKeyID, true, this->cached_info);
      }
    }

    generate_error_code_result_data(generate_data, error_code_file, error_id);
    return std::make_shared<PostLFRResponse>(request_id,  error_id);
  }
  catch (const std::exception & e)
  {
    Logger::Instance()->Error("Unspecified failure from DropManager handling drop request with error:\n{}", e.what());

    return std::make_shared<PostLFRResponse>(request_id, 10);
  }
  catch (...)
  {
    Logger::Instance()->Error("Unspecified failure from DropManager handling drop request ");

    return  std::make_shared<PostLFRResponse>(request_id, 10);
  }
}


std::pair<int, int> DropManager::handle_pre_post_compare(std::string PrimaryKeyID)
{
  std::shared_ptr<LfbVisionConfiguration> lfb_vision_config = this->mongo_bag_state->raw_mongo_doc->Config;
  std::shared_ptr<MarkerDetectorContainer> cached_pre_container = this->cached_pre_container;
  std::shared_ptr<MarkerDetectorContainer> cached_post_container = this->cached_post_container;
  std::shared_ptr<nlohmann::json> cached_pre_request_json = this->cached_pre_request;
  std::shared_ptr<nlohmann::json> cached_post_request_json = this->cached_post_request;
  std::shared_ptr<nlohmann::json> cached_drop_target_json = this->cached_drop_target_request;
  std::shared_ptr<cv::Point2f> cached_drop_target = this->cached_drop_target;
  std::shared_ptr<int> cached_drop_damage_code = this->cached_drop_damage_code;

  //confirm that all inputs for pre/post comparison have been cached properly and are available for use in algorithm
  std::string errors = "";
  if (cached_pre_container == nullptr) errors += "pre_container \n";
  if (cached_post_container == nullptr) errors += "post_container \n";
  if (cached_pre_request_json == nullptr) errors += "pre request json \n";
  if (cached_post_request_json == nullptr) errors += "post request json \n";
  if (cached_drop_target_json == nullptr) errors += "drop target json \n";
  if (cached_drop_target == nullptr) errors += "drop target \n";
  if (cached_drop_damage_code == nullptr) errors += "drop damage code \n";
  if (!errors.empty())
  {
    Logger::Instance()->Warn("The following inputs for pre/post comparison were not cached properly, cannot proceed: {}", errors);
    return std::pair<int, int>{-1, -1};
  }

  const int item_not_detected_code = 0;
  const int item_detected_code = 1;

  try
  {
    Logger::Instance()->Debug("Executing Pre/Post Comparison now");
    //this->session->lock(); //necessary to prevent Video_Generator interference with unaligned frames //TODO: is this necessary here? Everything is cached??
    std::shared_ptr<cv::Mat> result_mat = nullptr;
    int target_item_overlap = -1;
    int comparison_result = this->pre_post_compare->run_comparison(cached_pre_container, cached_post_container, lfb_vision_config,
                                           cached_pre_request_json, cached_post_request_json,
                                           cached_drop_target_json, *cached_drop_target, &result_mat, &target_item_overlap);

    Logger::Instance()->Debug("Percentage of item that landed on target is: {}", target_item_overlap);

    if(this->cached_info != nullptr and this->drop_live_viewer != nullptr)
    {

    if (comparison_result != item_not_detected_code and comparison_result != item_detected_code) this->cached_info->push_back("Item Detect Failed");

    std::string message;
    switch (comparison_result) {
      case (item_not_detected_code): message = "Item in bag: No"; break;
      case (item_detected_code): message = "Item in bag: Yes"; break;
      case (PrePostCompareErrorCodes::NotEnoughMarkersDetected): message = "Not Enough Markers"; break;
      case (PrePostCompareErrorCodes::InvalidItemDimensions): message = "Invalid Item Dimensions"; break;
      case (PrePostCompareErrorCodes::InvalidRequest): message = "Invalid Platform Value"; break;
      case (PrePostCompareErrorCodes::InconsistentPlatform): message = "Platform Inconsistency"; break;
      case (PrePostCompareErrorCodes::UnspecifiedError): message = "Undefined Failure"; break;
      default: message = "Unhandled case encountered"; break;
    }

    this->cached_info->push_back(message);

    //use cached info from dispense to create a live visualization of the info to be displayed on FDT
    this->drop_live_viewer->update_image(this->drop_live_viewer->get_blank_visualization(), ViewerImageType::Info, PrimaryKeyID, true, this->cached_info);
    }
    //send result to live_viewer for visualization purposes (comparison_result == nullptr if no items found)
    if (drop_live_viewer != nullptr)
    {
      if(result_mat != nullptr)
      {
        drop_live_viewer->update_image(drop_live_viewer->get_item_detection_visualization(result_mat), ViewerImageType::LFB_Item_Detection, PrimaryKeyID);
        if (comparison_result != item_not_detected_code and comparison_result != item_detected_code){
            Logger::Instance()->Info("Pre/Post Compare failed, but still updating live_viewer");
        }
      }
      else
      {
        Logger::Instance()->Info("No result item detection visualization available. Will instead upload default");
      }
    }

    //NOTE: comparison_result behavior kept the same because we handle case 3,4,5 by throwing exception here
    if (comparison_result != item_not_detected_code and comparison_result != item_detected_code){
        throw(comparison_result);
    }

    int item_drop_damage_type = *cached_drop_damage_code;
    if(this->mongo_bag_state == nullptr)
    {
      Logger::Instance()->Error("Mongo Bag State is nullptr, cannot proceed with updating the doc based on comparison results");
      return std::pair <int, int>{-1, -1};
    }
    this->mongo_bag_state->update_item_map(result_mat, item_drop_damage_type);

    //if the item was successfully dispensed, update the packed item volume state, based on the dropped item details
    if(comparison_result)
    {
      if(this->mongo_bag_state->packing_state == nullptr)
      {
        Logger::Instance()->Error("Mongo Bag State->Packing_State is nullptr, cannot proceed with updating packing state");
        return std::pair <int, int>{-1, -1};
      }
      else
      {
        this->mongo_bag_state->packing_state->update_packed_items_volume(cached_drop_target_json);
      }
    }
    return std::pair <int, int>{comparison_result, target_item_overlap};
  }
  catch (int error_id)
  {
    Logger::Instance()->Info("Pre-Post Comparison failed with error id: {}", error_id);
    return std::pair <int, int>{-1, -1};
  }
  catch (const std::exception & e)
  {
    Logger::Instance()->Error("Unspecified failure from Pre/Post Comparison handling:\n{}", e.what());
    return std::pair <int, int>{-1, -1};
  }
  catch (...)
  {
    Logger::Instance()->Error("Unspecified failure from Pre/Post Comparison handling");
    return std::pair <int, int>{-1, -1};
  }
}

std::vector<int> DropManager::check_products_for_fit_in_bag(std::shared_ptr<nlohmann::json> request_json)
{
    std::shared_ptr<LfbVisionConfiguration> lfb_vision_config = this->mongo_bag_state->raw_mongo_doc->Config;
    // TODO why is this assigned a shadowing name? Why assign to a new ptr at all? It does not appear to be copied or overwritten?
    std::shared_ptr<MarkerDetectorContainer> cached_post_container = this->cached_post_container;

    //confirm that all inputs have been cached properly and are available for use in this algorithm
    std::string errors = "";
    if (cached_post_container == nullptr) errors += "post_container \n";
    if (!errors.empty())
    {
        Logger::Instance()->Warn("The following inputs for check_products_for_fit_in_bag were not cached properly, cannot proceed: {}", errors);
        return std::vector<int>{};
    }

    try {
        Logger::Instance()->Debug("DropManager: Executing Check for Remaining Products Fit In Bag Now");
        auto products_to_check = (*request_json)["Remaining_Products_To_Pack"];
        Logger::Instance()->Info("DropManager: Check Products for Fit in Bag: found {} products to analyze", products_to_check.size());
        if (products_to_check.empty()) return std::vector<int>{}; //exit early if no products to check

        float remaining_platform = ((*request_json)["Remaining_Platform"].get<float>()) / 1000; // [meter] units
        Logger::Instance()->Debug("DropManager: Check Product Fit, creating drop grid now");
        DropGrid drop_depth_grid = DropGrid(cached_post_container->width, cached_post_container->length, lfb_vision_config->num_rows_in_drop_depth_grid, lfb_vision_config->num_cols_in_drop_depth_grid); //TODO (SB): add grid squares to config? 22, 15
        Logger::Instance()->Debug("DropManager: Check Product Fit, getting point cloud now");
        std::shared_ptr<LocalPointCloud> point_cloud = cached_post_container->get_point_cloud(false)->as_local_cloud();
        Logger::Instance()->Debug("DropManager: Check Product Fit, populating depth grid now");
        drop_depth_grid.populate_depth(point_cloud->get_data());

        std::vector<int> problematic_products;
        Logger::Instance()->Debug("DropManager: Check Product Fit, cycling through list of products now");
        for (auto p = products_to_check.begin(); p != products_to_check.end(); ++p)
        {
            auto product = p.value();

            int product_id = product["ProductId"].get<int>();
            float product_length = product["L"].get<float>()/1000; // [meter] units
            float product_width = product["W"].get<float>()/1000; // [meter] units
            float product_height = product["H"].get<float>()/1000; // [meter] units

            //product_length in tray is equivalent to shadow_height in bag (assumption) for these calculations
            float max_item_length_percent_overflow = lfb_vision_config->max_item_length_percent_overflow;
            float acceptable_height_above_marker_surface = std::min((max_item_length_percent_overflow * product_length), acceptable_Z_above_marker_surface); //[meter] units
            float upper_depth_limit = remaining_platform - product_length + acceptable_height_above_marker_surface;
            //note how product dimensions in tray translate to the shadow inputs for this function (width --> width, height --> shadow length)
            bool fit_result = drop_depth_grid.check_whether_item_fits(product_width, product_height, upper_depth_limit);
            Logger::Instance()->Debug("Bag fit check: product {}, with dimensions (L, W, H) {}, {}, {} has fit: {}", product_id, product_length, product_width, product_height, fit_result);
            if(!fit_result) problematic_products.push_back(product_id);
        }
        return problematic_products;
    }
    catch (...)
    {
        Logger::Instance()->Error("Unspecified failure from Check Products For Fit In Bag algo");
        return std::vector<int>{};
    }
}



// ***** ALL SIDE DISPENSE-SPECIFIC FUNCTIONALITY FOUND BELOW *****
std::shared_ptr<SideDropResult> DropManager::handle_pre_side_dispense_request(std::shared_ptr<std::string> request_id,
                                                        std::shared_ptr<std::string> primary_key_id,
                                                        std::shared_ptr<nlohmann::json> request_json,
                                                        std::shared_ptr<std::string> base_directory_input,
                                                        std::shared_ptr<std::string> time_stamp_string,
                                                        bool generate_data)
{
    Logger::Instance()->Debug("Starting DropManager::PreSideDispense for " + *primary_key_id);
    auto timer = fulfil::utils::timing::Timer("DropManager::handle_pre_side_dispense_request for " + *primary_key_id);

// needs data destination of 
    std_filesystem::path base_directory = make_media::paths::join_as_path(
    (base_directory_input) ? *base_directory_input : "","Side_Bag_Camera", *primary_key_id);
    std::string error_code_file = ("Side_Pre_Drop_Image" / base_directory / *time_stamp_string / "error_code").string();
    Logger::Instance()->Debug("Base directory is {}", base_directory.string());

    try {
        auto bag_state = this->mongo_bag_state->GetStateAsJson();
        Logger::Instance()->Debug("Bag state is currently: {}", bag_state.dump());

        std::shared_ptr<DataGenerator> generator;
        std::string data_destination = (base_directory / "Side_Pre_Drop_Image").string();
        generate_request_handling_data(generate_data,
            base_directory,
            time_stamp_string,
            request_json,
            std::make_shared<nlohmann::json>(bag_state));

        // get color image for live viewer
        if (this->drop_live_viewer != nullptr) this->drop_live_viewer->update_image(
             this->session->get_color_mat(), ViewerImageType::LFB_Pre_Dispense, *primary_key_id);

        // depth & marker container
        std::shared_ptr<LfbVisionConfiguration> lfb_vision_config = this->mongo_bag_state->raw_mongo_doc->Config;
        Logger::Instance()->Debug("LfbVisionConfiguration Generation: {}", lfb_vision_config->lfb_generation);

        std::shared_ptr<SideDropResult> result = this->searcher->handle_pre_side_dispense(request_id, primary_key_id, request_json, lfb_vision_config);
        this->cached_pre_container = result->container; //cache container for potential use in prepostcomparison later
        this->cached_pre_request = request_json;

        Logger::Instance()->Debug("Occupancy map returned from searcher with content TODO: {}", 0);
        generate_error_code_result_data(generate_data, error_code_file, result->success_code);
        return result;
    }
    catch (const rs2::unrecoverable_error& e)
    {
        std::string error_descrip = std::string("Realsense Exception: `") + e.what() +
                             std::string("`\nIn function: `") + e.get_failed_function() +
                             std::string("`\nWith args: `") + e.get_failed_args() + std::string("`");
        Logger::Instance()->Fatal(error_descrip);
        return std::make_shared<SideDropResult>(request_id, nullptr, SideDispenseErrorCodes::UnrecoverableRealSenseError, error_descrip);
    }
    catch (const rs2::recoverable_error& e)
    {
        std::string error_msg = std::string("Realsense Exception: `") + e.what() +
                         std::string("`\nIn function: `") + e.get_failed_function() +
                         std::string("`\nWith args: `") + e.get_failed_args() + std::string("`");
        Logger::Instance()->Error(error_msg);
        return std::make_shared<SideDropResult>(request_id, nullptr, SideDispenseErrorCodes::RecoverableRealSenseError, error_msg);
    }
    catch (const std::invalid_argument& e)
    {
        std::string error_description = std::string("Invalid Argument Exception: ") + e.what();
        Logger::Instance()->Error(error_description);
        return std::make_shared<SideDropResult>(request_id, nullptr, SideDispenseErrorCodes::NoMarkersDetected, error_description);
    }
    // TODO these catch statements should just be realsense and general exceptions right?
    catch (SideDispenseError & e)
    {
        SideDispenseErrorCodes error_id = e.get_status_code();
        Logger::Instance()->Info("DropManager failed handling drop request: {}", e.what());

        // TODO: should the occupancy map be written here?
        return std::make_shared<SideDropResult>(request_id, nullptr, error_id, e.get_description());
    }
    // TODO - remove. this is horrible. sincerely, jess
    catch (std::tuple<int, std::string> & e)
    {
        SideDispenseErrorCodes error_id = (SideDispenseErrorCodes)std::get<int>(e);
        std::string error_desc = std::get<std::string>(e);
        Logger::Instance()->Info("DropManager failed handling drop request: {}", error_desc);

        // TODO: should the occupancy map be written here?
        //generate_drop_target_result_data(generate_data, target_file, error_code_file, -1, -1, error_id);
        return std::make_shared<SideDropResult>(request_id, nullptr, error_id, error_desc);
    }
    catch (const std::exception &e) {
        Logger::Instance()->Error("Unspecified failure from DropManager handling drop request with error:\n{}",
                                  e.what());
        return std::make_shared<SideDropResult>(request_id, nullptr, SideDispenseErrorCodes::UnspecifiedError, e.what());
    }
    catch (...) {
        Logger::Instance()->Error("Unspecified failure from DropManager handling drop request in catch(...) block");
        return std::make_shared<SideDropResult>(request_id, nullptr, SideDispenseErrorCodes::UnspecifiedError, "In catch(...) block");
    }
}

std::shared_ptr<SideDropResult> DropManager::handle_post_side_dispense_request(std::shared_ptr<std::string> request_id,
                                                        std::shared_ptr<std::string> primary_key_id,
                                                        std::shared_ptr<nlohmann::json> request_json,
                                                        std::shared_ptr<std::string> base_directory_input,
                                                        std::shared_ptr<std::string> time_stamp_string,
                                                        bool generate_data)
{
    Logger::Instance()->Debug("Starting DropManager::PostSideDispense for " + *primary_key_id);
    auto timer = fulfil::utils::timing::Timer("DropManager::handle_pre_side_dispense_request for " + *primary_key_id);

    std_filesystem::path base_directory = make_media::paths::join_as_path(
    (base_directory_input) ? *base_directory_input : "","Side_Bag_Camera", *primary_key_id);
    std::string error_code_file = ("Side_Post_Drop_Image" / base_directory / *time_stamp_string / "error_code").string();
    Logger::Instance()->Debug("Base directory is {}", base_directory.string());
    // /home/fulfil/Videos/saved_images_2024_11_04/Side_Bag_Camera/671a8c713c760011e22754e6

    try {
        auto bag_state = this->mongo_bag_state->GetStateAsJson();
        Logger::Instance()->Debug("Bag state is currently: {}", bag_state.dump());

        std::shared_ptr<DataGenerator> generator;
        std::string data_destination = (base_directory / "Side_Post_Drop_Image").string();
        generate_request_handling_data(generate_data,
            base_directory,
            time_stamp_string,
            request_json,
            std::make_shared<nlohmann::json>(bag_state));

        // get color image for live viewer
        if (this->drop_live_viewer != nullptr) this->drop_live_viewer->update_image(
             this->session->get_color_mat(), ViewerImageType::LFB_Post_Dispense, *primary_key_id);

        // get lfb vision config
        std::shared_ptr<LfbVisionConfiguration> lfb_vision_config = this->mongo_bag_state->raw_mongo_doc->Config;
        Logger::Instance()->Debug("LfbVisionConfiguration Generation: {}", lfb_vision_config->lfb_generation);

        std::shared_ptr<SideDropResult> result = this->searcher->handle_post_side_dispense(request_id, primary_key_id, request_json, lfb_vision_config);
        this->cached_post_container = result->container; // cache container for potential use in prepostcomparison later
        this->cached_post_request = request_json;

        return std::make_shared<SideDropResult>(request_id, nullptr, SideDispenseErrorCodes::Success, std::string(""));
    }
    catch (const rs2::unrecoverable_error& e)
    {
        std::string error_descrip = std::string("Realsense Exception: `") + e.what() +
                             std::string("`\nIn function: `") + e.get_failed_function() +
                             std::string("`\nWith args: `") + e.get_failed_args() + std::string("`");
        Logger::Instance()->Fatal(error_descrip);
        return std::make_shared<SideDropResult>(request_id, nullptr, SideDispenseErrorCodes::UnrecoverableRealSenseError, error_descrip);
    }
    catch (const rs2::recoverable_error& e)
    {
        std::string error_msg = std::string("Realsense Exception: `") + e.what() +
                         std::string("`\nIn function: `") + e.get_failed_function() +
                         std::string("`\nWith args: `") + e.get_failed_args() + std::string("`");
        Logger::Instance()->Error(error_msg);
        return std::make_shared<SideDropResult>(request_id, nullptr, SideDispenseErrorCodes::RecoverableRealSenseError, error_msg);
    }
    catch (const std::invalid_argument& e)
    {
        std::string error_description = std::string("Invalid Argument Exception: ") + e.what();
        Logger::Instance()->Error(error_description);
        return std::make_shared<SideDropResult>(request_id, nullptr, SideDispenseErrorCodes::NoMarkersDetected, error_description);
    }
    // TODO these catch statements should just be realsense and general exceptions right?
    catch (SideDispenseError & e)
    {
        SideDispenseErrorCodes error_id = e.get_status_code();
        Logger::Instance()->Info("DropManager failed handling drop request: {}", e.what());

        return std::make_shared<SideDropResult>(request_id, nullptr, error_id, e.get_description());
    }
    // TODO - remove. this is horrible. sincerely, jess
    catch (std::tuple<int, std::string> & e)
    {
        SideDispenseErrorCodes error_id = (SideDispenseErrorCodes)std::get<int>(e);
        std::string error_desc = std::get<std::string>(e);
        Logger::Instance()->Info("DropManager failed handling post side request: {}", error_desc);
        //generate_drop_target_result_data(generate_data, target_file, error_code_file, -1, -1, error_id);
        return std::make_shared<SideDropResult>(request_id, nullptr, error_id, error_desc);
    }
    catch (const std::exception &e) {
        Logger::Instance()->Error("Unspecified failure from DropManager handling post side request with error:\n{}",
                                  e.what());
        return std::make_shared<SideDropResult>(request_id, nullptr, SideDispenseErrorCodes::UnspecifiedError, e.what());
    }
    catch (...) {
        Logger::Instance()->Error("Unspecified failure from DropManager handling post side request in catch(...) block");
        return std::make_shared<SideDropResult>(request_id, nullptr, SideDispenseErrorCodes::UnspecifiedError, "In catch(...) block");
    }
}
