#include <memory>
#include <tuple>
#include <vector>
#include <Fulfil.CPPUtils/eigen.h>
#include <Fulfil.CPPUtils/eigen/matrix3d_predicate.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.DepthCam/point_cloud.h>
#include <Fulfil.DepthCam/visualization.h>
#include "Fulfil.Dispense/dispense/dispense_manager.h"
#include "Fulfil.Dispense/dispense/drop_error_codes.h"
#include <Fulfil.Dispense/drop/drop_grid.h>
#include <Fulfil.Dispense/drop/drop_result.h>
#include <Fulfil.Dispense/drop/drop_zone_searcher.h>
#include <Fulfil.Dispense/visualization/live_viewer.h>

using fulfil::depthcam::aruco::MarkerDetector;
using fulfil::depthcam::aruco::Container;
using fulfil::depthcam::aruco::MarkerDetectorContainer;
using fulfil::depthcam::pointcloud::LocalPointCloud;
using fulfil::depthcam::pointcloud::PixelPointCloud;
using fulfil::depthcam::pointcloud::PointCloud;
using fulfil::depthcam::Session;
using fulfil::depthcam::visualization::SessionVisualizer;
using fulfil::dispense::commands::DropTargetDetails;
using fulfil::dispense::drop_target_error_codes::DropTargetErrorCodes;
using fulfil::dispense::drop_target_error_codes::DropTargetError;
using fulfil::dispense::drop::DropGrid;
using fulfil::dispense::drop::DropResult;
using fulfil::dispense::drop::DropZoneSearcher;
using fulfil::dispense::visualization::LiveViewer;
using fulfil::dispense::visualization::ViewerImageType;
using fulfil::utils::Logger;
using fulfil::utils::Point3D;


DropZoneSearcher::DropZoneSearcher(std::shared_ptr<Session> session,
                                   int visualize,
                                   int force_error,
                                   float shadow_buffer,
                                   int item_mass_threshold,
                                   std::shared_ptr<LiveViewer> drop_live_viewer,
                                   int debug, int min_bag_filtering_threshold, float white_region_depth_adjust_from_min,
                                    int empty_bag_threshold,
                                   float interference_depth_tolerance, int num_interference_points_tolerance,
                                  float interference_region_length_factor,
                                   bool visualize_interference_zone, float max_acceptable_Z_above_marker_surface,
                                   float significant_variance_regression, float moderate_variance_regression,
                                   float moderate_variance_improvement, float significant_variance_improvement,
                                   float significant_depth_regression,
                                   float equivalent_depth, float moderate_depth_improvement, float significant_depth_improvement,
                                   float crazy_depth_regression, float crazy_depth_improvement)
{
  this->session = session;
  this->debug = debug;
  this->visualize = visualize;
  this->force_error = force_error;
  //Global variables for affecting algorithm
  this->shadow_buffer = shadow_buffer;
  this->item_mass_threshold = item_mass_threshold;
  this->drop_live_viewer = drop_live_viewer;
  this->min_bag_filtering_threshold = min_bag_filtering_threshold;
  this->white_region_depth_adjust_from_min = white_region_depth_adjust_from_min;
  this->empty_bag_threshold = empty_bag_threshold;

  this->interference_depth_tolerance = interference_depth_tolerance;
  this->num_interference_points_tolerance = num_interference_points_tolerance;
  this->interference_region_length_factor = interference_region_length_factor;
  this->visualize_interference_zone = visualize_interference_zone;
  this->max_acceptable_Z_above_marker_surface = max_acceptable_Z_above_marker_surface;
  this->acceptable_height_above_marker_surface = max_acceptable_Z_above_marker_surface;

  this->significant_variance_regression = significant_variance_regression;
  this->moderate_variance_regression = moderate_variance_regression;
  this->moderate_variance_improvement = moderate_variance_improvement;
  this->significant_variance_improvement = significant_variance_improvement;
  this->crazy_depth_regression = crazy_depth_regression;
  this->significant_depth_regression = significant_depth_regression;
  this->equivalent_depth = equivalent_depth;
  this->moderate_depth_improvement = moderate_depth_improvement;
  this->significant_depth_improvement = significant_depth_improvement;
  this->crazy_depth_improvement = crazy_depth_improvement;

  if(interference_depth_tolerance <= white_region_depth_adjust_from_min)
  {
    Logger::Instance()->Error("Interference depth tolerance is below white region adjustment. Check ini file, this will lead to strange behavior!");
  }
  /**
  *   Setup Visualizations + Windows for Displaying and Debug
  */
  std::shared_ptr<std::pair <int, int>> location_top_left = std::make_shared<std::pair <int, int>>(0,0);
  std::shared_ptr<std::pair <int, int>> location_top_right = std::make_shared<std::pair <int, int>>(1000, 0);
  std::shared_ptr<std::pair <int, int>> location_bottom_left = std::make_shared<std::pair <int, int>>(0,720);
  std::shared_ptr<std::pair <int, int>> location_bottom_right = std::make_shared<std::pair <int, int>>(1000,720);

  std::shared_ptr<std::pair <int, int>> window_size = std::make_shared<std::pair <int, int>>(960,540); // 960, 540   //1280,  720 for one monitor,  960, 540  for laptop
  std::shared_ptr<std::pair <int, int>> window_size2 = std::make_shared<std::pair <int, int>>(250,325);

  int no_wait = 1;
  int yes_wait = 0;

  std::shared_ptr<std::string> window_name_1 = std::make_shared<std::string>("Window 1: Raw RGB Image");
  std::shared_ptr<std::string> window_name_2 = std::make_shared<std::string>("Window 2: Detected Markers");
  std::shared_ptr<std::string> window_name_3 = std::make_shared<std::string>("Window 3: Container Cavity Depth Data");
  std::shared_ptr<std::string> window_name_4 = std::make_shared<std::string>("Window 4: Adjusted Cavity Depth Data");
  std::shared_ptr<std::string> window_name_5 = std::make_shared<std::string>("Window 5: Drop Center Candidates");
  std::shared_ptr<std::string> window_name_6 = std::make_shared<std::string>("Window 6: Drop Target");
  std::shared_ptr<std::string> window_name_7 = std::make_shared<std::string>("Window 7: Bag Filtering");
  std::shared_ptr<std::string> window_name_8 = std::make_shared<std::string>("Window 8: Interference Zone");
  std::shared_ptr<std::string> window_name_8andahalf = std::make_shared<std::string>("Window 8.5: Maximum Depth Points");
  std::shared_ptr<std::string> window_name_9 = std::make_shared<std::string>("Window 9: Candidates, post Damage Risk Filter");

  this->session_visualizer1 = std::make_shared<SessionVisualizer>(session, window_name_1, location_top_left, window_size, no_wait);
  this->session_visualizer2 = std::make_shared<SessionVisualizer>(session, window_name_2, location_top_left, window_size, no_wait);
  this->session_visualizer3 = std::make_shared<SessionVisualizer>(session, window_name_3, location_top_right, window_size,yes_wait);
  this->session_visualizer4 = std::make_shared<SessionVisualizer>(session, window_name_4, location_top_right, window_size, yes_wait);
  this->session_visualizer5 = std::make_shared<SessionVisualizer>(session, window_name_5, location_bottom_left, window_size, no_wait);
  this->session_visualizer6 = std::make_shared<SessionVisualizer>(session, window_name_6, location_bottom_right, window_size, no_wait);
  this->session_visualizer7 = std::make_shared<SessionVisualizer>(session, window_name_7, location_top_right, window_size, no_wait);
  this->session_visualizer8 = std::make_shared<SessionVisualizer>(session, window_name_8, location_bottom_right, window_size, yes_wait);
  this->session_visualizer8andahalf = std::make_shared<SessionVisualizer>(session, window_name_8andahalf, location_bottom_right, window_size, yes_wait);
  this->session_visualizer9 = std::make_shared<SessionVisualizer>(session, window_name_9, location_bottom_left, window_size, no_wait);
}

void DropZoneSearcher::check_inputs(float shadow_length,
                                    float shadow_width,
                                    float shadow_height,
                                    std::shared_ptr<INIReader> LFB_config_reader,
                                    std::shared_ptr<fulfil::dispense::commands::DropTargetDetails> details)
{
  float LFB_width = LFB_config_reader->GetFloat("LFB_config", "LFB_width", -1);
  float LFB_length = LFB_config_reader->GetFloat("LFB_config", "LFB_length", -1);

  float container_length = LFB_config_reader->GetFloat("LFB_config", "container_length", -1);
  float container_width = LFB_config_reader->GetFloat("LFB_config", "container_width", -1);

  if(shadow_length <= 0 || shadow_width <= 0 || shadow_height <= 0 )
  {
    Logger::Instance()->Error("Invalid Drop Item Dimensions Provided; Vars: Length = {}, Width = {}, Height = {}; Cam: LFB",
                              shadow_height, shadow_width, shadow_length);
    this->success_code = DropTargetErrorCodes::InvalidItemDimensions;
    throw DropTargetError(DropTargetErrorCodes::InvalidItemDimensions,
                          "Item dimensions provided: {Length = " + std::to_string(shadow_height) +
                          ", Width = " + std::to_string(shadow_width) +
                          ", Height = " + std::to_string(shadow_length) + "}");
  }

  if(details->bag_item_count < 0)
  {
    Logger::Instance()->Error("Invalid Bag Item Count; Vars: count = {}; Cam: LFB", details->bag_item_count);
  }

  Logger::Instance()->Info("Limits on target: Left = {}, Right = {}, Back = {}, Front = {}",
                            details->limit_left, details->limit_right, details->limit_back, details->limit_front);
  if(details->limit_left < 0 or details->limit_right < 0 or details->limit_front < 0 or details->limit_back < 0)
  {
    Logger::Instance()->Error("Invalid Limit value provided in pre-dispense request input, check json file for more info");
    this->success_code = DropTargetErrorCodes::InvalidBotSideLimitValue;
    throw DropTargetError(DropTargetErrorCodes::InvalidBotSideLimitValue,
                          "Limits provided: {Left = " + std::to_string(details->limit_left) +
                          ", Right = " + std::to_string(details->limit_right) +
                          ", Back = " + std::to_string(details->limit_back) +
                          ", Front = " + std::to_string(details->limit_front) + "}");
  }

  float half_LFB_width_mm = 1000*(LFB_width)/2.0;
  if(details->limit_left > half_LFB_width_mm or details->limit_right > half_LFB_width_mm)
  {
    Logger::Instance()->Warn("Limit left: {}, or Limit right: {} are greater than half the width of LFR: {}, check VLSG code",
                              details->limit_left, details->limit_right, half_LFB_width_mm);
  }

  if(details->item_mass <= 0.0)
  {
    Logger::Instance()->Error("Invalid Item Mass: {}", details->item_mass);
    this->success_code = DropTargetErrorCodes::InvalidItemMass;
    throw DropTargetError(DropTargetErrorCodes::InvalidItemMass,
                          "Item mass provided: " + std::to_string(details->item_mass));
  }

  if(details->remaining_platform > -0.010 and details->remaining_platform < 0.0)
  {
    Logger::Instance()->Warn("Remaining platform input is negative: {}. Within expected tolerance, setting to 0", details->remaining_platform);
    details->remaining_platform = 0;
  } else if(details->remaining_platform < -0.050  // TODO: how to not hardcode
          or details->remaining_platform > 0.400)  // TODO: where can these be defined and not hardcoded
  {
    Logger::Instance()->Error("Expected remaining_platform valid value for empty bag, instead received: {}", details->remaining_platform);
    this->success_code = DropTargetErrorCodes::InvalidRemainingPlatformValue;
    throw DropTargetError(DropTargetErrorCodes::InvalidRemainingPlatformValue,
                          "Remaining platform value provided: " + std::to_string(details->remaining_platform));
  }

  if(details->item_damage_code < 0 or details->item_damage_code > 25)
  {
    Logger::Instance()->Error("Invalid Item Material Code Provided in Json Request: {}", details->item_damage_code);
    this->success_code = DropTargetErrorCodes::InvalidItemMaterialCode;
    throw DropTargetError(DropTargetErrorCodes::InvalidItemMaterialCode,
                          "Item material code provided: " + std::to_string(details->item_damage_code));
  }

  Logger::Instance()->Debug("Container width: {}. Container Length: {},  shadow_width: {}. shadow_length: {}", container_width, container_length, shadow_width, shadow_length);
  if (shadow_width > container_width or shadow_length > container_length)
  {
    Logger::Instance()->Error("Input item dimensions are larger than LFB container dimensions!");
    this->success_code = DropTargetErrorCodes::ItemLargerThanBag;
    throw DropTargetError(DropTargetErrorCodes::ItemLargerThanBag,
                          "Item dimentions provided: {Width = " + std::to_string(shadow_width) +
                          ", Height = " + std::to_string(shadow_length) +
                          "}. Container dimensions provided: {Width = " + std::to_string(container_width) +
                          ", Length = " + std::to_string(container_length) +
                          "}. Keep in mind LFB width/length is flipped from FC to DCAPI.");
  }
}

std::shared_ptr<Point3D> DropZoneSearcher::get_empty_bag_target(std::shared_ptr<fulfil::dispense::commands::DropTargetDetails> details,
                                                                std::shared_ptr<INIReader> LFB_config_reader, float shadow_length, float shadow_width,
                                                                   float LFB_cavity_height)
{
  float container_length = LFB_config_reader->GetFloat("LFB_config", "container_length", -1);
  float container_width = LFB_config_reader->GetFloat("LFB_config", "container_width", -1);
  // TODO - do a deep rename of all orientation variables
  float port_edge_target_offset = LFB_config_reader->GetFloat("LFB_config", "port_edge_target_offset", 0);
  float item_protrusion_detection_threshold = LFB_config_reader->GetFloat("LFB_config", "item_protrusion_detection_threshold", 0.005);
  int depth_points_above_threshold_to_count_as_item_protruding = LFB_config_reader->GetInteger("LFB_config", "depth_points_above_threshold_to_count_as_item_protruding", 5);
  bool use_y_coordinates_orientation_check = LFB_config_reader->GetBoolean("LFB_config", "use_y_coordinates_orientation_check", false);

  Logger::Instance()->Debug("Bag Item Count is input as 0, target will be set at front left corner of bag");

  // negative Z is depth into bag, 0 is marker height, positive is above bag
  float target_Z = -1*(LFB_cavity_height - details->remaining_platform);
  float target_x;
  Logger::Instance()->Debug("This dispense does flip the X default to prefer the non-default side: {}", details->use_flipped_x_default);
  if (details->use_flipped_x_default)
  {
    // move default drop target to front side, accounting for front edge offset
    float front_edge_target_offset = LFB_config_reader->GetFloat("LFB_config", "front_edge_target_offset", 0);
    target_x = (container_width/2) - shadow_width/2 - front_edge_target_offset;
  } else {
    // move default drop target to rear side, accounting for rear edge offset
    float rear_edge_target_offset = LFB_config_reader->GetFloat("LFB_config", "rear_edge_target_offset", 0);
    target_x = -(container_width/2) + shadow_width/2 + rear_edge_target_offset;
  }
  // port edge is on y-axis, must offset it
  float target_y = (container_length/2) - shadow_length/2 - port_edge_target_offset;
  //TODO: consider applying edge target offset to general dispense targets as well if needed. Change other offset to only apply in X

  //adjust default target if there are limits on LFB travel
  float min_allowable_x = -(container_width/2) + float(details->limit_left)/1000;
  float max_allowable_x_limit = (container_width/2) - float(details->limit_right)/1000;
  float max_allowable_x = std::min(max_allowable_x_limit, container_width/2 - shadow_width/2);

  if(target_x < min_allowable_x)
  {
    Logger::Instance()->Warn("Adjusting Empty Bag Target due to Limit Left restrictions on LFB travel!");
    target_x = min_allowable_x;
  }
  if(target_x > max_allowable_x)
  {
    Logger::Instance()->Error("Cannot Adjust Empty Bag Target due to item dimensions and Right Limit on bot. Rejecting bot");
    this->success_code = DropTargetErrorCodes::NoViableTarget_ItemDimensionsIncompatibleWithBotLimits;
    throw DropTargetError(DropTargetErrorCodes::NoViableTarget_ItemDimensionsIncompatibleWithBotLimits,
                          "Cannot Adjust Empty Bag Target due to item dimensions and Right Limit on bot. Rejecting bot");
  }

  std::shared_ptr<Point3D> XYZ_result = std::make_shared<Point3D>(target_x,target_y,target_Z);  //see LFB diagram for explanation of
  return XYZ_result;
}

std::shared_ptr<std::vector<Point3D>> initialize_empty_depth_list(int amount_of_max_depth_points_to_track, float default_depth)
{
    std::vector<Point3D> empty_depth_list;
    for (int i = 0; i < amount_of_max_depth_points_to_track; i++)
    {
       empty_depth_list.push_back(Point3D(0,0,default_depth));
    }
    return std::make_shared<std::vector<Point3D>>(empty_depth_list);
}


std::shared_ptr<std::vector<Point3D>> update_max_depth_list(std::shared_ptr<std::vector<Point3D>> depth_list, float local_x, float local_y, float local_z)
{
//    bool shift_list_down = false;
//    int idx_of_new_depth_point = -1;
//    Point3D next_depth_point = Point3D(0,0,0);
    // the 0th index is greatest, last index is smallest
    for (int i = 0; i < depth_list->size(); i++) {
        // if the given depth point has been inserted earlier in the list, the rest of the list must be shifted over
//        if (shift_list_down) {
//            // is this actually shifting them?? or are they all 0s
////            depth_list.at(i) = next_depth_point;
//            Logger::Instance()->Debug("Idx being shifted: {}, with value: {}", i, std::to_string(depth_list.at(i-1).z));
//        }
        // if the depth point at curr index is smaller depth than the given depth point, the given depth point must be inserted into the list
        if (depth_list->at(i).z < local_z) {
            Logger::Instance()->Debug("Idx being inserted: {}, with old value: {}, and new value: {}", i, std::to_string(depth_list->at(i).z), local_z);
            depth_list->insert(depth_list->begin() + i, Point3D(local_x, local_y, local_z));
            Logger::Instance()->Debug("Next idx now: {}, with value: {}", i+1, std::to_string(depth_list->at(i+1).z));
            Logger::Instance()->Debug("Length after insert: {}", depth_list->size());
            depth_list->pop_back();
            break;
        }

//            shift_list_down = true;
//            next_depth_point = depth_list.at(i);
//            depth_list.at(idx_of_new_depth_point) = Point3D(local_x, local_y, local_z);
//            idx_of_new_depth_point = i;
        Logger::Instance()->Debug("Idx being utilized: {}, with local_z: {}", i, local_z);
    }
//    if (idx_of_new_depth_point != -1)
//    {
//        Logger::Instance()->Debug("Idx being set: {}, with local_z: {}", idx_of_new_depth_point, local_z);
//        depth_list.at(idx_of_new_depth_point) = Point3D(local_x, local_y, local_z);
//        Logger::Instance()->Debug("NEW POINT VALUE: {}", depth_list.at(idx_of_new_depth_point).z);
//    }
    return depth_list;
}

fulfil::utils::Point3D get_max_z_that_is_not_outlier(
//        Point3D max_depth_point,
                                                      float item_protrusion_detection_threshold,
                                                      std::shared_ptr<std::vector<fulfil::utils::Point3D>> depth_list,
                                                      float threshold_depth_distance_from_max,
                                                      int num_points_required_within_valid_distance_to_validate_max_z)
{
    Point3D max_depth_point = depth_list->at(0);
    if (max_depth_point.z > item_protrusion_detection_threshold)
    {
        int num_within_valid_distance_threshold = 0;
        int potential_max_idx = 0;
        // the 0th index is greatest, last index is smallest
        for (int i = 1; i < depth_list->size(); i++)
        {
            Logger::Instance()->Debug("Depth Point is (x: {}, y: {}, z: {})", depth_list->at(i).x, depth_list->at(i).y, depth_list->at(i).z);
            if (abs(max_depth_point.z - depth_list->at(i).z) <= threshold_depth_distance_from_max)
            {
                num_within_valid_distance_threshold++;
            } else {
                potential_max_idx = i;
            }
        }
        if (num_within_valid_distance_threshold >= num_points_required_within_valid_distance_to_validate_max_z)
        {
            // if there are enough nearby depth points to validate the max depth point, return it
            return max_depth_point;
        } else {
            // else return the first point not in the validation distance
            return depth_list->at(potential_max_idx);
//            return Point3D(depth_list->at(potential_max_idx).x, depth_list->at(potential_max_idx).y, std::max(0.0F, depth_list->at(potential_max_idx).z));
        }
    }
    return max_depth_point;
}

DropZoneSearcher::Max_Z_Points DropZoneSearcher::adjust_depth_detections(std::shared_ptr<cv::Mat> RGB_matrix, std::shared_ptr<LocalPointCloud> input_cloud,
                                                  float item_mass, float platform_in_LFB_coords, bool should_search_right_to_left, std::shared_ptr<INIReader> LFB_config_reader,
                                                  bool visualize_flag, bool live_viewer_flag, bool should_check_empty, bool force_adjustment)  //Todo: if widely applicable should move this function into implementations NoTranslationPointCloud, TranslatedPointCloud, Untranslated, DepthPixelpointCloud
{
  Logger::Instance()->Debug("Analyzing white regions of bag and finding max depth point. Min value thresh: {}",
                            this->min_bag_filtering_threshold);
  std::shared_ptr<Eigen::Matrix3Xd> local_cloud_data = input_cloud->get_data();
  std::shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>> pixels = input_cloud->as_pixel_cloud()->get_data();

  /**
   *   HSV filtering of RGB image
   */
  Logger::Instance()->Trace("temporary debug log: get hsv image");
  cv::Mat mask;
  cvtColor(*RGB_matrix, mask, cv::COLOR_BGR2GRAY);
  inRange(mask, this->min_bag_filtering_threshold, 255, mask);

  if(this->visualize == 1 and visualize_flag == 1) session_visualizer7->display_image(std::make_shared<cv::Mat>(mask));
  Logger::Instance()->Trace("temporary debug log: update live viewer image #7");
  if (this->drop_live_viewer != nullptr and live_viewer_flag == true) this->drop_live_viewer->update_image(std::make_shared<cv::Mat>(mask), ViewerImageType::LFB_Filter, this->PKID);

  Logger::Instance()->Trace("temporary debug log: start looping through pixels");

  // initialize struct for max detected depth points
  Max_Z_Points max_depth_points{Point3D(0,0,platform_in_LFB_coords), Point3D(0,0,platform_in_LFB_coords), Point3D(0,0,platform_in_LFB_coords),
                                Point3D(0,0,platform_in_LFB_coords), Point3D(0,0,platform_in_LFB_coords),
                                Point3D(0,0,platform_in_LFB_coords), Point3D(0,0,platform_in_LFB_coords),
                                Point3D(0,0,platform_in_LFB_coords), Point3D(0,0,platform_in_LFB_coords),
                                Point3D(0,0,platform_in_LFB_coords), Point3D(0,0,platform_in_LFB_coords),
                                Point3D(0,0,platform_in_LFB_coords), Point3D(0,0,platform_in_LFB_coords),
                                Point3D(0,0,platform_in_LFB_coords), 0};
  int white_count = 0; //for tracking how many depth points are recognized as white bag
  int non_white_count = 0; //for tracking how many depth points are not recognized as white bag

  float adjustment = LFB_config_reader->GetFloat("LFB_config", "fraction_of_bag_dims_considered_inner_bag", 0);
  float container_width = LFB_config_reader->GetFloat("LFB_config", "container_width", 0.43);
  float container_length = LFB_config_reader->GetFloat("LFB_config", "container_length", 0.30);
  bool should_filter_out_white = LFB_config_reader->GetBoolean("LFB_config", "should_filter_out_white", false);

  float item_protrusion_detection_threshold = LFB_config_reader->GetFloat("LFB_config", "item_protrusion_detection_threshold", 0.005);

  // inner bag limits
  float x_limit = adjustment*container_width/2;
  float y_limit = adjustment*container_length/2;

  //    antenna location is max_container_x + 15mm-20mm
  float lfb_width = LFB_config_reader->GetFloat("LFB_config", "LFB_width", -1);
  float antenna_x_coord_nominal = lfb_width / 2 + 0.0175;
  //    antenna location is min_container_y + 65mm-75mm
  float lfb_length = LFB_config_reader->GetFloat("LFB_config", "LFB_length", -1);

  float antenna_y_coord_nominal = lfb_length / -2 + 0.070;
  float antenna_buffer = 0.005;
  int amount_of_max_depth_points_to_track = 5;

  auto is_antenna_data = [antenna_x_coord_nominal, antenna_y_coord_nominal, antenna_buffer](float local_x_coord, float local_y_coord)-> bool
  {
      // if the given coords are within antenna_buffer meters from antenna coords
      return abs(local_x_coord) - antenna_x_coord_nominal < antenna_buffer and abs(local_y_coord) + antenna_y_coord_nominal < antenna_buffer;
  };
  std::shared_ptr<std::vector<Point3D>> outer_front_left_depth_list = initialize_empty_depth_list(amount_of_max_depth_points_to_track, platform_in_LFB_coords);
  std::shared_ptr<std::vector<Point3D>> outer_front_right_depth_list = initialize_empty_depth_list(amount_of_max_depth_points_to_track, platform_in_LFB_coords);
  std::shared_ptr<std::vector<Point3D>> outer_back_left_depth_list = initialize_empty_depth_list(amount_of_max_depth_points_to_track, platform_in_LFB_coords);
  std::shared_ptr<std::vector<Point3D>> outer_back_right_depth_list = initialize_empty_depth_list(amount_of_max_depth_points_to_track, platform_in_LFB_coords);

  bool should_adjust_depth = (item_mass > this->item_mass_threshold) or force_adjustment;
  if(should_adjust_depth)
  {
    Logger::Instance()->Debug("Adjusting white regions to platform in LFB coords: {:.3f} + depth offset: {:.3f}", platform_in_LFB_coords, this->white_region_depth_adjust_from_min);
  }
  else
  {
    Logger::Instance()->Info("Item mass is below threshold and no force depth adjust indicated. Will still count white points but not adjust depth values.");
  }

    // TODO - total revamp of this function. optimize for computational speeds
    // TODO - account for bag crumpling, and rename this function because it doesn't adjust any depths in bagless bags

  // forward cycle left to right if not should_search_right_to_left
  // backward cycle right to left if should_search_right_to_left
  int pixels_size = pixels->size();
  int current_pixel_index = (should_search_right_to_left) ? pixels_size-1 : 0;
  auto is_still_looping = [should_search_right_to_left, &pixels_size, &current_pixel_index]()-> bool {
    return (should_search_right_to_left) ? current_pixel_index >= 0 : current_pixel_index < pixels_size;
  };
  auto next_step = [should_search_right_to_left, &current_pixel_index]()  {
      (should_search_right_to_left) ? --current_pixel_index : ++current_pixel_index;
  };
    //cycle through all points in bag and log the max depth points in each region, while adjusting depth of white pixels
    for (; is_still_looping(); next_step()) {
      cv::Point2f pixel = *pixels->at(current_pixel_index);
      int white_intensity = mask.at<uchar>(round(pixel.y), round(pixel.x));
      float local_x = (*local_cloud_data)(0, current_pixel_index);
      float local_y = (*local_cloud_data)(1, current_pixel_index);
      float local_z = (*local_cloud_data)(2, current_pixel_index);

      bool is_outer_bag = (local_x < -x_limit) or (local_x > x_limit) or (local_y < -y_limit) or (local_y > y_limit);
      // count number of white points for empty/nonempty bag analysis
      if (white_intensity == 255) {
          // only care about white count of inner bag, not if the LFB walls are white
          if (is_outer_bag) { white_count += 1; }
          // only filter on whiteness if feature flag is enabled
          if (should_filter_out_white) { continue; }
          // TODO - this is disabled from adjusting the white points in a bag to platform height. needs to be refactored for different bags vs bagless.
          //only adjust depths if item is above a certain threshold mass
          //if(should_adjust_depth and is_outer_bag) input_cloud->set_depth_value(i, platform_in_LFB_coords + this->white_region_depth_adjust_from_min);
      } else {
          non_white_count += 1;
      }
      if (!is_outer_bag) {
          if (local_y >= 0 && local_x >= 0) {
              if (local_z > max_depth_points.front_right.z) {
                  max_depth_points.front_right.x = local_x;
                  max_depth_points.front_right.y = local_y;
                  max_depth_points.front_right.z = local_z;
              }
          } else if (local_y >= 0 && local_x < 0) {
              if (local_z > max_depth_points.front_left.z) {
                  max_depth_points.front_left.x = local_x;
                  max_depth_points.front_left.y = local_y;
                  max_depth_points.front_left.z = local_z;
              }
          } else if (local_y < 0 && local_x >= 0) {
              if (local_z > max_depth_points.back_right.z) {
                  max_depth_points.back_right.x = local_x;
                  max_depth_points.back_right.y = local_y;
                  max_depth_points.back_right.z = local_z;
              }
          } else {
              if (local_z > max_depth_points.back_left.z) {
                  max_depth_points.back_left.x = local_x;
                  max_depth_points.back_left.y = local_y;
                  max_depth_points.back_left.z = local_z;
              }
          }
      // if this IS outer bag
      } else {
          if (local_y >= 0 && local_x >= 0) {
              if (local_z > item_protrusion_detection_threshold) {
                  outer_front_right_depth_list = update_max_depth_list(outer_front_right_depth_list, local_x, local_y, local_z);
              }
          } else if (local_y >= 0 && local_x < 0) {
              if (local_z > item_protrusion_detection_threshold) {
                  outer_front_left_depth_list = update_max_depth_list(outer_front_left_depth_list, local_x, local_y, local_z);
              }
          } else if (local_y < 0 && local_x >= 0) {
              // do not include antenna in max depth detections
              if (is_antenna_data(local_x, local_y)) { continue; }
                  if (local_z > item_protrusion_detection_threshold) {
                      outer_back_right_depth_list = update_max_depth_list(outer_back_right_depth_list, local_x, local_y, local_z);
                  }
          } else {
              if (local_z > item_protrusion_detection_threshold) {
                  outer_back_left_depth_list = update_max_depth_list(outer_back_left_depth_list, local_x, local_y, local_z);
              }
          }
      }
  }

  // noise filtering of outer quadrant data
  max_depth_points.outer_front_left = get_max_z_that_is_not_outlier(
                                item_protrusion_detection_threshold,
                                outer_front_left_depth_list,
                                0.003,
                                5);
  max_depth_points.outer_front_right = get_max_z_that_is_not_outlier(
                                item_protrusion_detection_threshold,
                                outer_front_right_depth_list,
                                0.003,
                                5);
  max_depth_points.outer_back_left = get_max_z_that_is_not_outlier(
                                item_protrusion_detection_threshold,
                                outer_back_left_depth_list,
                                0.003,
                                5);
  max_depth_points.outer_back_right = get_max_z_that_is_not_outlier(
                                item_protrusion_detection_threshold,
                                outer_back_right_depth_list,
                                0.003,
                                5);

  max_depth_points.front = max_depth_points.front_left.z >= max_depth_points.front_right.z ? max_depth_points.front_left : max_depth_points.front_right;
  max_depth_points.back = max_depth_points.back_left.z >= max_depth_points.back_right.z ? max_depth_points.back_left : max_depth_points.back_right;
  max_depth_points.overall = max_depth_points.front.z >= max_depth_points.back.z ? max_depth_points.front : max_depth_points.back;
  max_depth_points.outer_front = max_depth_points.outer_front_right.z >= max_depth_points.outer_front_left.z ? max_depth_points.outer_front_right : max_depth_points.outer_front_left;
  max_depth_points.outer_back = max_depth_points.outer_back_right.z >= max_depth_points.outer_back_left.z ? max_depth_points.outer_back_right : max_depth_points.outer_back_left;
  max_depth_points.outer_overall = max_depth_points.outer_front.z >= max_depth_points.outer_back.z ? max_depth_points.outer_front : max_depth_points.outer_back;

  Logger::Instance()->Debug("Front region max depth pt: ({}, {}, {})", max_depth_points.front.x, max_depth_points.front.y, max_depth_points.front.z );
  Logger::Instance()->Debug("Front-Left region max depth pt: ({}, {}, {})", max_depth_points.front_left.x, max_depth_points.front_left.y, max_depth_points.front_left.z );
  Logger::Instance()->Debug("Front-Right region max depth pt: ({}, {}, {})", max_depth_points.front_right.x, max_depth_points.front_right.y, max_depth_points.front_right.z );
  Logger::Instance()->Debug("Back region max depth pt: ({}, {}, {})", max_depth_points.back.x, max_depth_points.back.y, max_depth_points.back.z );
  Logger::Instance()->Debug("Back-Left region max depth pt: ({}, {}, {})", max_depth_points.back_left.x, max_depth_points.back_left.y, max_depth_points.back_left.z );
  Logger::Instance()->Debug("Back-Right region max depth pt: ({}, {}, {})", max_depth_points.back_right.x, max_depth_points.back_right.y, max_depth_points.back_right.z );
  Logger::Instance()->Debug("Overall bag max depth pt: ({}, {}, {})", max_depth_points.overall.x, max_depth_points.overall.y, max_depth_points.overall.z );
  Logger::Instance()->Debug("Outer Front region max depth pt: ({}, {}, {})", max_depth_points.outer_front.x, max_depth_points.outer_front.y, max_depth_points.outer_front.z );
  Logger::Instance()->Debug("Outer Front-Left region max depth pt: ({}, {}, {})", max_depth_points.outer_front_left.x, max_depth_points.outer_front_left.y, max_depth_points.outer_front_left.z );
  Logger::Instance()->Debug("Outer Front-Right region max depth pt: ({}, {}, {})", max_depth_points.outer_front_right.x, max_depth_points.outer_front_right.y, max_depth_points.outer_front_right.z );
  Logger::Instance()->Debug("Outer Back region max depth pt: ({}, {}, {})", max_depth_points.outer_back.x, max_depth_points.outer_back.y, max_depth_points.outer_back.z );
  Logger::Instance()->Debug("Outer Back-Left region max depth pt: ({}, {}, {})", max_depth_points.outer_back_left.x, max_depth_points.outer_back_left.y, max_depth_points.outer_back_left.z );
  Logger::Instance()->Debug("Outer Back-Right region max depth pt: ({}, {}, {})", max_depth_points.outer_back_right.x, max_depth_points.outer_back_right.y, max_depth_points.outer_back_right.z );
  Logger::Instance()->Debug("Outer Overall bag max depth pt: ({}, {}, {})", max_depth_points.outer_overall.x, max_depth_points.outer_overall.y, max_depth_points.outer_overall.z );

  Eigen::Matrix3Xd max_z_eigen_data (3,4);
  max_z_eigen_data.col(0) << max_depth_points.outer_front_left.x,  max_depth_points.outer_front_left.y,  max_depth_points.outer_front_left.z;
  max_z_eigen_data.col(1) << max_depth_points.outer_front_right.x,  max_depth_points.outer_front_right.y,  max_depth_points.outer_front_right.z;
  max_z_eigen_data.col(2) << max_depth_points.outer_back_left.x,  max_depth_points.outer_back_left.y,  max_depth_points.outer_back_left.z;
  max_z_eigen_data.col(3) << max_depth_points.outer_back_right.x,  max_depth_points.outer_back_right.y,  max_depth_points.outer_back_right.z;

  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> max_z_local_point_cloud = input_cloud->as_local_cloud()->new_point_cloud(std::make_shared<Eigen::Matrix3Xd>(max_z_eigen_data));
  session_visualizer8andahalf->display_points(max_z_local_point_cloud, 0, 255, 0, 5, 3);
  std::stringstream  max_z_data_ss ; max_z_data_ss << max_z_eigen_data;
  Logger::Instance()->Debug("Max Z Matrix:\n{}", max_z_data_ss.str());

  int total_points = pixels->size();
  int percentage_white;
  if (total_points == 0)
  {
    percentage_white = 100;
  }
  else
  {
    percentage_white = int(100 * white_count / float(total_points));
  }
  Logger::Instance()->Debug("{}% of points are white, or {} out of a total of {} points", percentage_white, white_count, total_points);

  if(should_check_empty)
  {
    Logger::Instance()->Warn("Checking that bag is actually empty, as indicated during Drop Target Request");
    Logger::Instance()->Debug("Over {}% of points need to be white for bag to be indicated as empty", this->empty_bag_threshold);
    if(percentage_white < this->empty_bag_threshold) //TODO: move this to configs if needed long term. Probably not necessary
    {
      Logger::Instance()->Error("Bag Not Empty; Cam: LFB");
      this->success_code = DropTargetErrorCodes::EmptyBagNotEmpty;
      this->error_description = "Percentage white = " + std::to_string(percentage_white) + ", which is < the empty bag threshold value of " + std::to_string(this->empty_bag_threshold);
    }
  }
  return max_depth_points;
}

std::shared_ptr<LocalPointCloud> DropZoneSearcher::apply_box_limits(std::shared_ptr<MarkerDetectorContainer> container,
                                                                    std::shared_ptr<LocalPointCloud> point_cloud,
                                                                    float shadow_length, float shadow_width, float shadow_height,
                                                                    int limit_left, int limit_right, int limit_front, int limit_back,
                                                                    float remaining_platform, float LFB_width, float LFB_length,
                                                                    float container_width, float container_length, float front_edge_target_offset,
                                                                    float port_edge_target_offset,
                                                                    int bot_is_rotated)
{
  Logger::Instance()->Debug("container width is: {}  meters ", container_width);
  Logger::Instance()->Debug("container length is: {}  meters ", container_length);

  //Getting dimensions of box of valid drop zones, based on item size. These x and y coordinates are in bot local frame
  // THIS ALGORITHM ASSUMES THAT ALL POINTS IN THE BAG ARE REACHABLE BY EXTENDS OF DISPENSE CONVEYORS + ABILITY OF BOT TO ROTATE 180 DEGREES
  float min_target_x = -container_width/2 + shadow_width/2 + front_edge_target_offset;
  float max_target_x = container_width/2 - shadow_width/2 - front_edge_target_offset;
  float min_target_y = -container_length/2 + shadow_length/2 + port_edge_target_offset; //see limit diagrams for explanation
  float max_target_y = container_length/2 - shadow_length/2 - port_edge_target_offset;
  Logger::Instance()->Debug("Target candidate limits are set based on bot and item dimensions. min_y: {}, max_y: {}, min_x: {}, max_x: {}", min_target_y, max_target_y, min_target_x, max_target_x);

  if( (max_target_y < min_target_y) or (max_target_x < min_target_x) )
  {
    Logger::Instance()->Error("Target limits do not appear to make sense. Check request input and algorithm calculations");
    this->success_code = DropTargetErrorCodes::InvalidRequest;
    this->error_description ="Target limits do not appear to make sense. Check request input and algorithm calculations";
    throw DropTargetError(DropTargetErrorCodes::InvalidRequest, this->error_description);
  }

  float upper_depth_limit = float(remaining_platform - shadow_height) + this->acceptable_height_above_marker_surface; //assuming item lands perfectly vertical, need to be able to lower enough to make acceptable height above marker surface
  Logger::Instance()->Debug("Remaining platform is: {}, shadow height is: {}, so upper_depth_limit is: {}", remaining_platform, shadow_height, upper_depth_limit);

  //the line beyond which the target requires bot rotation from nominal to be reached.
  this->rotation_limit_line = -LFB_length/2 + ((float)limit_back)/1000 - shadow_length/2;
  Logger::Instance()->Debug("Target y coordinate beyond which rotation is required to reach target: {}", this->rotation_limit_line);

  //Predicate for filtering out non-viable targets
  std::shared_ptr<fulfil::utils::eigen::Matrix3dPredicate>
      predicate = std::make_shared<fulfil::utils::eigen::CustomMatrix3dPredicate>(
      [min_target_x, max_target_x, min_target_y, max_target_y, upper_depth_limit](const fulfil::utils::eigen::Matrix3dPoint &point) {
        return point(0) >= min_target_x  && point(0) <= max_target_x
            && point(1) >= min_target_y && point(1) <= max_target_y
            && point(2) <= upper_depth_limit; });

  std::shared_ptr<fulfil::utils::eigen::Matrix3XdFilter> filter;
  filter = std::make_shared<fulfil::utils::eigen::Matrix3XdFilter>(predicate);
  point_cloud->apply_filter(filter);  //this function is defined in translated_point_cloud.cpp
  return point_cloud;
}


DropZoneSearcher::Target_Region DropZoneSearcher::analyze_target_region(float shadow_length, float shadow_width, Point3D center,
                                           std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud, Point3D max_Z_point)
{
  float depth_sum = 0;
  std::vector<float> valid_depths {};

  int point_count = 0;

  float minx = center.x - shadow_width/2.0 - this->shadow_buffer;
  float maxx = center.x + shadow_width/2.0 + this->shadow_buffer;
  float miny = center.y - shadow_length/2.0 - this->shadow_buffer;
  float maxy = center.y + shadow_length/2.0 + this->shadow_buffer;

  std::shared_ptr<Eigen::Matrix3Xd> cloud_data = point_cloud->as_local_cloud()->get_data();

  for(int i = 0; i < cloud_data->cols(); i++)
  {
    float local_x = (*cloud_data)(0, i);
    float local_y = (*cloud_data)(1, i);
    float local_z = (*cloud_data)(2, i);

    if(local_x >= minx && local_x <= maxx && local_y >= miny && local_y <= maxy) //check point is within target region
    {
      depth_sum += local_z;
      valid_depths.push_back(local_z); //store all depth values for variance processing later
      point_count++;
    }
  }

  float mean = depth_sum / point_count;

  float variance_sum = 0;
  for (auto i = 0; i < valid_depths.size(); i++) { variance_sum += pow(mean - valid_depths[i], 2);  }

  float variance = 1000000*(variance_sum / point_count);

  float distance_to_max_Z_point = std::sqrt(pow(center.x - max_Z_point.x, 2) + pow(center.y - max_Z_point.y, 2));

  struct Target_Region target_region {};
  /* The average of the depth of the target region */
  target_region.average_depth = mean;
  /* The variance of the depth of the target region */
  target_region.variance_depth = variance;
  /* The maximum z coordinate in the target region */
  target_region.max_Z = *max_element(valid_depths.begin(), valid_depths.end());
  /* The minimum z coordinate in the target region */
  target_region.min_Z = *min_element(valid_depths.begin(), valid_depths.end());
  /* The raw x coordinate of the center point */
  target_region.x = center.x;
  /* The raw y coordinate of the center point */
  target_region.y = center.y;
  /* Use the raw Z  of the center point, not using halfway between average and max Z in target region anymore as that erroneously rejects bags for being too full */
  target_region.z = mean;
  /* The range of depths in the target region between the maximum and minimum depth points*/
  target_region.range_depth = target_region.max_Z - target_region.min_Z;
  /* The distance from the center point to the point with the max Z coordinate value */
  target_region.distance_to_max_LFB_Z = distance_to_max_Z_point;

  return target_region;
}


DropZoneSearcher::Interference_Region DropZoneSearcher::define_interference_region(float item_length, float shadow_width,
                                                                                   float item_height, Point3D center, bool target_requires_rotation_from_nominal)
{
  float shadow_length = item_height;

  float w2 = shadow_width;
  float d = std::sqrt(pow(item_height, 2) + pow(this->interference_region_length_factor * item_length, 2));

  struct Interference_Region inter_region;

  float region_length = d - shadow_length;
  float region_width = w2;

  float region_center_x = center.x;

  float y_offset = shadow_length/2.0 + region_length/2.0;
  //direction of interference region relative to target region depends on if LFB will rotate or not and current rotation status
  float region_center_y;

  region_center_y = target_requires_rotation_from_nominal ? center.y + y_offset : center.y - y_offset;

  Point3D interference_center = Point3D(region_center_x, region_center_y, center.z);

  inter_region.center = interference_center;
  inter_region.width = region_width;
  inter_region.length = region_length;

  return inter_region;
}


void DropZoneSearcher::check_interference(std::shared_ptr<DropZoneSearcher::Target_Region> target_region, std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud)
{
  Interference_Region interference_region = target_region->interference_region;
  float center_x = interference_region.center.x;
  float center_y = interference_region.center.y;
  float center_z = interference_region.center.z;

  float minx = center_x - interference_region.width/2.0;
  float maxx = center_x + interference_region.width/2.0;
  float miny = center_y - interference_region.length/2.0;
  float maxy = center_y + interference_region.length/2.0;
  float minz = center_z + this->interference_depth_tolerance;

  std::shared_ptr<Eigen::Matrix3Xd> cloud_data = point_cloud->as_local_cloud()->get_data();

  float interference_max_z = -1000;
  int num_interference_points = 0;

  float sum_interference_z = 0;
  for(int i = 0; i < cloud_data->cols(); i++)
  {
    float local_x = (*cloud_data)(0, i);
    float local_y = (*cloud_data)(1, i);
    float local_z = (*cloud_data)(2, i);

    if(local_x >= minx && local_x <= maxx && local_y >= miny && local_y <= maxy && local_z > minz) //check point is within interference region
    {
      sum_interference_z += local_z;
      num_interference_points++;
      if (local_z > interference_max_z) interference_max_z = local_z;
    }
  }
  if (num_interference_points >= this->num_interference_points_tolerance)
  {
    target_region->interference_detected = true;
    target_region->interference_max_z = interference_max_z;
    target_region->interference_average_z = sum_interference_z / num_interference_points;
    Logger::Instance()->Trace("Interference Threshold was met. {} points found, with maxz: {} and average z: {}",
                              num_interference_points, target_region->interference_max_z, target_region->interference_average_z);
  }
}


bool DropZoneSearcher::compare_candidates(std::shared_ptr<DropZoneSearcher::Target_Region> best_target_region, std::shared_ptr<DropZoneSearcher::Target_Region> current_target_region)
{
  bool interference_improved = ((best_target_region->interference_detected == true) and (current_target_region->interference_detected == false));
  bool rotation_improved = ((best_target_region->rotation_required == true) and (current_target_region->rotation_required == false));
  bool rotation_worsened = ((best_target_region->rotation_required == false) and (current_target_region->rotation_required == true));

  float min_diff = best_target_region->min_Z - current_target_region->min_Z;
  float max_diff = best_target_region->max_Z - current_target_region->max_Z;
  float average_diff = best_target_region->average_depth - current_target_region->average_depth;
  float variance_diff = best_target_region->variance_depth - current_target_region->variance_depth;
  float range_diff = best_target_region->range_depth - current_target_region->range_depth;
  float distance_to_max_LFB_Z_diff = best_target_region->distance_to_max_LFB_Z - current_target_region->distance_to_max_LFB_Z;

  bool significant_variance_regression = variance_diff < this->significant_variance_regression;
  bool moderate_variance_regression = variance_diff < this->moderate_variance_regression;
  bool moderate_variance_improvement = variance_diff > this->moderate_variance_improvement;
  bool significant_variance_improvement = variance_diff > this->significant_variance_improvement;

  bool crazy_depth_regression = (average_diff < this->crazy_depth_regression);
  bool significant_depth_regression = (average_diff < this->significant_depth_regression); // significantly worse depth
  bool equivalent_depth = (average_diff > this->equivalent_depth); // equivalent or better average depth
  bool moderate_depth_improvement = (average_diff > this->moderate_depth_improvement); //modest or better average depth
  bool significant_depth_improvement = (average_diff > this->significant_depth_improvement); //significantly better average depth
  bool crazy_depth_improvement = (average_diff > this->crazy_depth_improvement);

  if(rotation_worsened)
  {
    if(crazy_depth_improvement)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else if (rotation_improved)
  {
    if(!crazy_depth_regression)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else //rotation of candidates is the same
  {
    if (interference_improved and !significant_depth_regression)
    {
      return true;
    }
    else if (significant_depth_improvement and !significant_variance_regression)
    {
      return true;
    }
    else if (moderate_depth_improvement and !moderate_variance_regression)
    {
      return true;
    }
    else if (equivalent_depth and moderate_variance_improvement)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
}

std::shared_ptr<cv::Mat> DropZoneSearcher::visualize_target(std::shared_ptr<Point3D> result, std::shared_ptr<fulfil::depthcam::aruco::MarkerDetectorContainer> container, float shadow_length, float shadow_width, std::shared_ptr<cv::Mat> RGB_matrix, int color, int thickness)
{
  std::shared_ptr<Eigen::Matrix3Xd> new_local_data = std::make_shared<Eigen::Matrix3Xd>(3, 4);
  // Bottom right corner of drop zone
  (*new_local_data)(0,0) = result->x - shadow_width/2;
  (*new_local_data)(1,0) = result->y - shadow_length/2;
  (*new_local_data)(2,0) = result->z;
  // Top right corner of drop zone
  (*new_local_data)(0,1) = result->x - shadow_width/2;
  (*new_local_data)(1,1) = result->y + shadow_length/2;
  (*new_local_data)(2,1) = result->z;
  // Top left corner of drop zone
  (*new_local_data)(0,2) = result->x + shadow_width/2;
  (*new_local_data)(1,2) = result->y + shadow_length/2;
  (*new_local_data)(2,2) = result->z;
  // Bottom left corner of drop zone.
  (*new_local_data)(0,3) = result->x + shadow_width/2;
  (*new_local_data)(1,3) = result->y - shadow_length/2;
  (*new_local_data)(2,3) = result->z;

  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud2 =  container->get_point_cloud(false);
  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> target_result;
  target_result = point_cloud2->as_local_cloud()->new_point_cloud(new_local_data);

  std::shared_ptr<cv::Mat> RGB_matrix_copy = std::make_shared<cv::Mat>(RGB_matrix->clone());
  std::shared_ptr<cv::Mat> output_image = session_visualizer6->display_perimeter(target_result, RGB_matrix_copy, color, thickness);
  return output_image;
}


bool sort_ascending_y(const std::tuple<float, float, float>& a,
               const std::tuple<float, float, float>& b)
{
  return (std::get<1>(a) > std::get<1>(b));
}


void eigen_sort_columns_by_y(std::shared_ptr<Eigen::Matrix3Xd> matrix)
{
  std::vector<std::tuple<float, float, float>> tuples_vec;

  for (int i = 0; i < matrix->cols(); i++)
  {
    auto column = matrix->col(i);
    std::tuple<float, float, float> tuple = std::make_tuple(column(0), column(1), column(2));
    tuples_vec.push_back(tuple);
  }

  sort(tuples_vec.begin(), tuples_vec.end(), sort_ascending_y);

  for (int i = 0; i < matrix->cols(); i++)
  {
    (*matrix)(0, i) = std::get<0>(tuples_vec[i]);
    (*matrix)(1, i) = std::get<1>(tuples_vec[i]);
    (*matrix)(2, i) = std::get<2>(tuples_vec[i]);
  }
};

std::shared_ptr<MarkerDetectorContainer> DropZoneSearcher::get_container(std::shared_ptr<INIReader> LFB_config_reader, std::shared_ptr<Session> session, bool extend_region_over_markers)
{
  int region_max_x = LFB_config_reader->GetInteger("LFB_config", "region_max_x", -1);
  int region_min_x = LFB_config_reader->GetInteger("LFB_config", "region_min_x", -1);
  int region_max_y = LFB_config_reader->GetInteger("LFB_config", "region_max_y", -1);
  int region_min_y = LFB_config_reader->GetInteger("LFB_config", "region_min_y", -1);

  float marker_adjust_amount = LFB_config_reader->GetFloat("LFB_config", "marker_adjust_amount", 0.0);

  int num_markers = LFB_config_reader->GetInteger("LFB_config", "num_markers", -1);
  int marker_size = LFB_config_reader->GetInteger("LFB_config", "marker_size", -1);

  float marker_depth = LFB_config_reader->GetFloat("LFB_config", "marker_depth", -1);
  float marker_depth_tolerance = LFB_config_reader->GetFloat("LFB_config", "marker_depth_tolerance", -1);

  float container_length = LFB_config_reader->GetFloat("LFB_config", "container_length", -1);
  float container_width = LFB_config_reader->GetFloat("LFB_config", "container_width", -1);
  float lfb_width = LFB_config_reader->GetFloat("LFB_config", "LFB_width", -1);
  float lfb_length = LFB_config_reader->GetFloat("LFB_config", "LFB_length", -1);

  std::shared_ptr<Eigen::Matrix3Xd> marker_coordinates = std::shared_ptr<Eigen::Matrix3Xd>(new Eigen::Matrix3Xd(3,8));
  std::vector<float> dims; // num_calib_coordinates x height x width x depth
  std::string section = "LFB_config";
  LFB_config_reader->FillFloatVector(section, "marker_coordinates_x", dims);
  for (int i = 0; i < 8; i++) (*marker_coordinates)(0, i) = dims[i];
  dims.clear();
  LFB_config_reader->FillFloatVector(section, "marker_coordinates_y", dims);
  for (int i = 0; i < 8; i++) (*marker_coordinates)(1, i) = dims[i];
  dims.clear();
  LFB_config_reader->FillFloatVector(section, "marker_coordinates_z", dims);
  for (int i = 0; i < 8; i++) (*marker_coordinates)(2, i) = dims[i];


  Logger::Instance()->Trace("Drop Zone Searcher: marker detector constructor called");
  std::shared_ptr<MarkerDetector> marker_detector = std::make_shared<MarkerDetector>(num_markers, marker_size);

  Logger::Instance()->Trace("Drop Zone Searcher: marker detector container constructor called");
  std::shared_ptr<MarkerDetectorContainer> container = std::make_shared<MarkerDetectorContainer>(
      marker_detector, session, true, extend_region_over_markers,
      container_width, container_length, lfb_width, lfb_length,
      MarkerDetectorContainer::centers_and_sides(), marker_coordinates,
      num_markers, marker_depth, marker_depth_tolerance,
      region_max_x, region_min_x, region_max_y, region_min_y, marker_adjust_amount);

  return container;
}

int DropZoneSearcher::check_bot_rotated(std::vector<std::shared_ptr<fulfil::depthcam::aruco::Marker>> markers, bool use_y_coordinates)
{
  int num_markers = markers.size();
  if(num_markers < 3)
  {
    Logger::Instance()->Error("Unexpected input to Check_Bot_Rotated function: not enough markers ({})! Need at least 3", num_markers);
    this->success_code = DropTargetErrorCodes::NotEnoughMarkersDetected;
    throw DropTargetError(DropTargetErrorCodes::NotEnoughMarkersDetected,
                          "Number of markers detected: " + std::to_string(num_markers)); //This should never happen
  }

  Logger::Instance()->Warn("Checking orientation of bot using y pixel coordinates for LFB3: {}", use_y_coordinates);

  int left_side_index_1 = -1;
  int right_side_index_1 = -1;

  int first_id = -1;
  int first_pixel = -1;
  int second_id = -1;
  int second_pixel = -1;

  auto log_and_return = [](int detected_rotation) {
      if (detected_rotation == 1) {
          Logger::Instance()->Info(
            "Check Rotation: Bot was detected as rotated 180 degrees from nominal, special considerations will take place");
          return detected_rotation;
      }
      Logger::Instance()->Debug("Check Rotation: Bot was detected in nominal rotation");
      return detected_rotation;

  };

  //find two markers on the same side of the LFB to compare their pixel coordinates
  // x pixel coordinates as defined by OpenCV convention is +X going from left to right in the image (used for LFB2)
  // y pixel coordinates as defined by OpenCV convention is +Y going from top to bottom in image (used for LFB3)
      // when grab y pixel coordinates, flip the sign to make the calculation uniform for both
  //TODO: when permanently switch to LFB3s, remove X pixel functionality and boolean input to the function
  for(int i = 0; i < 3; i++)
  {
    fulfil::depthcam::aruco::Marker marker = *markers.at(i);
    int marker_id = marker.get_id();

    if (marker_id == 1 or marker_id == 2 or marker_id == 3 or marker_id == 4) {
      if (right_side_index_1 == -1)
      {
        right_side_index_1 = i;
      }
      else //found a second marker on that same side
      {
        first_id = markers.at(right_side_index_1)->get_id();
        second_id = marker_id;
        if(use_y_coordinates)
        {
          first_pixel = -1.0 * markers.at(right_side_index_1)->get_coordinate(fulfil::depthcam::aruco::Marker::center)->y;
          second_pixel = -1.0 * marker.get_coordinate(fulfil::depthcam::aruco::Marker::center)->y;
        }
        else
        {
          first_pixel = markers.at(right_side_index_1)->get_coordinate(fulfil::depthcam::aruco::Marker::center)->x;
          second_pixel = marker.get_coordinate(fulfil::depthcam::aruco::Marker::center)->x;
        }
        break;
      }
    }
    else
    {
      if (left_side_index_1 == -1)
      {
        left_side_index_1 = i;
      }
      else //found a second marker on that same side
      {
        first_id = markers.at(left_side_index_1)->get_id();
        second_id = marker_id;

        if(use_y_coordinates)
        {
          first_pixel = -1.0 * markers.at(left_side_index_1)->get_coordinate(fulfil::depthcam::aruco::Marker::center)->y;
          second_pixel = -1.0 * marker.get_coordinate(fulfil::depthcam::aruco::Marker::center)->y;
        }
        else
        {
          first_pixel = markers.at(left_side_index_1)->get_coordinate(fulfil::depthcam::aruco::Marker::center)->x;
          second_pixel = marker.get_coordinate(fulfil::depthcam::aruco::Marker::center)->x;
        }
        break;
      }
    }
  }

  Logger::Instance()->Debug("Check bot rotation. Marker1: {}, Marker2: {}, pixel_1: {}, pixel_2: {}",
                            first_id, second_id, first_pixel, second_pixel);
  switch (first_id)
  {
    case 1 ... 4:
      if ((first_id > second_id) && (first_pixel >= second_pixel)) return  log_and_return(0);
      if ((first_id > second_id) && (first_pixel < second_pixel)) return  log_and_return(1);
      if ((first_id < second_id) && (first_pixel >= second_pixel)) return  log_and_return(1);
      if ((first_id < second_id) && (first_pixel < second_pixel)) return  log_and_return(0);
      break;
    case 5 ... 7:
      if(second_id == 0)
      {
        if (first_pixel >= second_pixel) return  log_and_return(0);
        if (first_pixel < second_pixel) return  log_and_return(1);
      }
      else
      {
        if ((first_id > second_id) && (first_pixel >= second_pixel)) return  log_and_return(1);
        if ((first_id > second_id) && (first_pixel < second_pixel)) return  log_and_return(0);
        if ((first_id < second_id) && (first_pixel >= second_pixel)) return  log_and_return(0);
        if ((first_id < second_id) && (first_pixel < second_pixel)) return  log_and_return(1);
      }
      break;
    case 0:
      if (first_pixel >= second_pixel) return  log_and_return(1);
      if (first_pixel < second_pixel) return  log_and_return(0);
    default:
      break;
  }
  Logger::Instance()->Error("Error in figuring out rotation status, returning -1");
  return -1;
}

void DropZoneSearcher::validate_marker_positions(bool nominal_bot_rotation, std::vector<std::shared_ptr<fulfil::depthcam::aruco::Marker>> markers,
                               std::shared_ptr<INIReader> LFB_config_reader)
{
  Logger::Instance()->Debug("Conducting second round validation check on marker locations");
  int min_dim_1 = LFB_config_reader->GetInteger("LFB_config", "min_dim_1", -1);
  int max_dim_1 = LFB_config_reader->GetInteger("LFB_config", "max_dim_1", -1);
  int min_dim_2 = LFB_config_reader->GetInteger("LFB_config", "min_dim_2", -1);
  int max_dim_2 = LFB_config_reader->GetInteger("LFB_config", "max_dim_2", -1);
  bool check_y_pixel_coordinates = LFB_config_reader->GetBoolean("LFB_config", "extra_check_in_y_coordinates", false);

  std::set<int> marker_set_1 = {1, 2, 3, 4};
  std::set<int> marker_set_2 = {0, 5, 6, 7};

  int min_dim_marker_set_1 = nominal_bot_rotation ? min_dim_1 : min_dim_2;
  int max_dim_marker_set_1 = nominal_bot_rotation ? max_dim_1 : max_dim_2;
  int min_dim_marker_set_2 = nominal_bot_rotation ? min_dim_2 : min_dim_1;
  int max_dim_marker_set_2 = nominal_bot_rotation ? max_dim_2 : max_dim_1;

  for(int i = 0; i < markers.size(); i++)
  {
    fulfil::depthcam::aruco::Marker marker = *markers.at(i);
    int marker_id = marker.get_id();
    bool marker_in_set_1 = marker_set_1.find(marker_id) != marker_set_1.end();

    int min_bound_check = marker_in_set_1 ? min_dim_marker_set_1 : min_dim_marker_set_2;
    int max_bound_check = marker_in_set_1 ? max_dim_marker_set_1 : max_dim_marker_set_2;
    int x_pixel = marker.get_coordinate(fulfil::depthcam::aruco::Marker::center)->x;
    int y_pixel = marker.get_coordinate(fulfil::depthcam::aruco::Marker::center)->y;

    Logger::Instance()->Debug("Extra validation check, marker: {}, x pixel: {}, y pixel: {}",
                              marker_id, x_pixel, y_pixel);

    int value_to_compare = check_y_pixel_coordinates ? y_pixel : x_pixel;

    if(value_to_compare < min_bound_check || value_to_compare > max_bound_check)
    {
      Logger::Instance()->Error("Marker failed extra bound check; Marker number: {}, Value: {}, Min Bound: {}, Max Bound: {}, y_coordinate: {}",
                                marker_id, value_to_compare, min_bound_check, max_bound_check, check_y_pixel_coordinates);
      this->success_code = DropTargetErrorCodes::UnexpectedBagPosition;
      throw DropTargetError(DropTargetErrorCodes::UnexpectedBagPosition,
                            "Marker failed extra bound check. Marker ID: " + std::to_string(marker_id) +
                            ", Value: " + std::to_string(value_to_compare) +
                            ", Min Bound: " + std::to_string(min_bound_check) +
                            ", Max Bound: " + std::to_string(max_bound_check) +
                            ", y_coordinate: " + std::to_string(check_y_pixel_coordinates));
    }
  }
}

std::shared_ptr<DropResult> DropZoneSearcher::find_drop_zone_center(std::shared_ptr<MarkerDetectorContainer> container,
                                                               std::shared_ptr<DropTargetDetails> details,
                                                               std::shared_ptr<INIReader> LFB_config_reader,
                                                               std::shared_ptr<mongo::MongoBagState> mongo_bag_state,
                                                               bool bot_has_already_rotated)

{
  Logger::Instance()->Debug("find_drop_zone_center called in drop_zone_searcher");
  Logger::Instance()->Debug("Bag item count is: {}", details->bag_item_count);
  Logger::Instance()->Debug("Mass threshold is: {}", this->item_mass_threshold);

  this->success_code = DropTargetErrorCodes::Success; //default code is success, can be changed later if there are errors along the way

  if (this->force_error == 1)
  {
    Logger::Instance()->Error("Forced Error was thrown in Drop_Zone_Searcher, check main.ini drop_zone_searcher config");
    this->success_code = DropTargetErrorCodes::AlgorithmFail_Bypass;
    throw DropTargetError(DropTargetErrorCodes::AlgorithmFail_Bypass,
                          "Error forced by `force_error` being true");
  }

  float LFB_width = LFB_config_reader->GetFloat("LFB_config", "LFB_width", -1);
  float LFB_length = LFB_config_reader->GetFloat("LFB_config", "LFB_length", -1);
  float LFB_bag_width = LFB_config_reader->GetFloat("LFB_config", "LFB_bag_width", -1);
  float LFB_bag_length = LFB_config_reader->GetFloat("LFB_config", "LFB_bag_length", -1);
  float container_width = LFB_config_reader->GetFloat("LFB_config", "container_width", -1);
  float container_length = LFB_config_reader->GetFloat("LFB_config", "container_length", -1);
  float front_edge_target_offset = LFB_config_reader->GetFloat("LFB_config", "front_edge_target_offset", 0);
  float port_edge_target_offset = LFB_config_reader->GetFloat("LFB_config", "port_edge_target_offset", 0);
  bool prefer_targets_farther_from_max_depth = LFB_config_reader->GetBoolean("LFB_config", "prefer_targets_farther_from_max_depth", false);

  bool use_y_coordinates_orientation_check =  LFB_config_reader->GetBoolean("LFB_config", "use_y_coordinates_orientation_check", false);

  float LFB_cavity_height = LFB_config_reader->GetFloat("LFB_config", "LFB_cavity_height", -1);

  Logger::Instance()->Trace("Drop Zone Search Algorithm Initiated");

  float shadow_length = details->item_height;
  float shadow_width = details->item_width;
  float shadow_height = details->item_length;

  check_inputs(shadow_length, shadow_width, shadow_height, LFB_config_reader, details);

  /**
   * Establish if bot rotation is allowed when considering drop target candidates. TODO: move into separate function
   */
  bool rotation_allowed_by_configs = LFB_config_reader->GetBoolean("LFB_config", "rotation_allowed", false);
  if(rotation_allowed_by_configs)
  {
    if(!bot_has_already_rotated)
    {
      Logger::Instance()->Debug("Bot rotation is allowed in configs and bot is able to rotate further");
      this->LFB_rotation_allowed = true;
    }
    else
    {
      Logger::Instance()->Warn("Bot rotation is allowed in configs but bot has already rotated once for this dispense");
      this->LFB_rotation_allowed = false;
    }
  }
  else
  {
    Logger::Instance()->Warn("Bot rotation is not allowed in configs");
    this->LFB_rotation_allowed = false;
  }

  //define amount that items can stick out above marker surface (AFTER DISPENSE) as minimum of 45% of item length and the max allowable by config setting). Values are in meters
  float max_item_length_percent_overflow = LFB_config_reader->GetFloat("LFB_config", "max_item_length_percent_overflow", 0.45);
  this->acceptable_height_above_marker_surface = std::min((max_item_length_percent_overflow * shadow_height), this->max_acceptable_Z_above_marker_surface);
  Logger::Instance()->Debug("Shadow height is {} and acceptable height above marker surface is {}", shadow_height, this->acceptable_height_above_marker_surface);

  std::shared_ptr<cv::Mat> RGB_matrix = container->get_color_mat();
  if(this->visualize == 1) session_visualizer1->display_rgb_image(RGB_matrix);
  if (this->drop_live_viewer != nullptr) this->drop_live_viewer->update_image(RGB_matrix, ViewerImageType::LFB_RGB, this->PKID);

  // Visualization for detected valid markers
  std::shared_ptr<std::vector<std::shared_ptr<fulfil::depthcam::aruco::Marker>>> markers = container->get_markers(!bot_has_already_rotated);
  std::shared_ptr<cv::Mat> marker_visualization = session_visualizer2->draw_detected_markers(container->marker_detector->dictionary, markers, container->region_max_x, container->region_min_x, container->region_max_y, container->region_min_y);

  if (this->drop_live_viewer != nullptr) this->drop_live_viewer->update_image(marker_visualization, ViewerImageType::LFB_Markers, this->PKID);
  if(this->visualize == 1) session_visualizer2->display_rgb_image(marker_visualization);

  std::shared_ptr<LocalPointCloud> point_cloud = container->get_point_cloud(false)->as_local_cloud();

  int original_candidate_count = point_cloud->get_data()->cols();

  std::shared_ptr<cv::Mat> image3 = session_visualizer3->display_points_with_depth_coloring(point_cloud);
  if(this->visualize == 1) session_visualizer3->display_image(image3);
  if (this->drop_live_viewer != nullptr) this->drop_live_viewer->update_image(image3, ViewerImageType::LFB_Depth, this->PKID);

  // Determine if bot is rotated or not for Pirouette-related purposes
  /**
   * Establish if bot rotation is allowed when considering drop target candidates.
   * TODO: figure out wtf is going on. The clause below using rotation status from request was at the top. and then there
   * is this. For now I am going to use the input param to validate the rotation check
   * next major time this is used is near `Visualize and find best target from the valid candidates` comment
   */
  int bot_is_rotated = check_bot_rotated(*markers, use_y_coordinates_orientation_check);
  if(bot_is_rotated == -1) {
      this->success_code = DropTargetErrorCodes::CouldNotDetermineBotRotationStatus;
      throw DropTargetError(DropTargetErrorCodes::CouldNotDetermineBotRotationStatus);
  }

  /**
   *  Additional check on marker positions for bot being centered under camera, if indicated by configs
   */
  const float extra_marker_validation_check = LFB_config_reader->GetBoolean("LFB_config", "extra_marker_validation_required", false);
  if(extra_marker_validation_check) { validate_marker_positions(!bot_is_rotated, *markers, LFB_config_reader); }
  const float mass_threshold_extra_fragile = LFB_config_reader->GetFloat("LFB_config", "mass_threshold_extra_fragile", 150.0);


  /**
   *  Mongo bag state functionality starts here //TODO: refactor into standalone function(s) once tested
   */
  //try to get current item map from mongo bag state
  bool risk_map_flag = false; //flag to keep track of whether risk map is available and needs consideration when filtering
  std::shared_ptr<cv::Mat> risk_map_ptr = nullptr;

  if(mongo_bag_state == nullptr)
  {
    Logger::Instance()->Warn("mongo_bag_state is nullptr in drop_zone_searcher->find_drop_zone_center method. Unexpected behavior!!");
  } else {
    try
    {
      int damage_type = details->item_damage_code;
      bool avoid_all_items = (damage_type == 21); //cannot drop extra fragile items on any other item
      std::vector<int> risk_materials {}; // vector of damage types that cannot be dispensed on top of for this damage type item

      if(!avoid_all_items)
      {
        // Todo .... what is going on here? if damage is 21, can't we not get here?
        // Add ExtraFragile items to damage Risk group for dispense, unless item is very light
        // TODO: align this value w/ MassThresholdExtraFragile BagPackingRecipe in Mongo eventually
        if (details->item_mass < mass_threshold_extra_fragile) {
            Logger::Instance()->Debug(
              "Dispense allowed onto ExtraFragile items because item mass is: {}", details->item_mass);
        } else{
            risk_materials.push_back(21); // cannot drop onto ExtraFragile items
        }
        // Add items to damage risk group based on damage types
        switch (damage_type)
        {
          case 4:
            risk_materials.push_back(4);    // cannot drop glass items onto glass items
            break;
          case 3:
            if (LFB_config_reader->GetBoolean("LFB_config", "avoid_metal_on_metal", true)) {
                risk_materials.push_back(3);    // can't allow metal on metal...? I think?
            } else {
                Logger::Instance()->Info("Skip running damage avoidance for metal on metal");
            }
          default:
            break;
        }
      }
      Logger::Instance()->Info("Dispense item damage code is: {}, mass is: {}. Avoiding all items: {}", damage_type, details->item_mass, avoid_all_items);
      for(int & risk_material : risk_materials)
      {
        Logger::Instance()->Info("Will not drop on items of damage type: {}", risk_material);
      }

      int layers_to_include = LFB_config_reader->GetInteger("LFB_config", "damage_layers_to_include");

      if( (risk_materials.size() != 0 or avoid_all_items) and details->bag_item_count > 0)
      {
          Logger::Instance()->Debug("Drop contains damage risk for certain materials! Getting risk map now");
          risk_map_ptr = mongo_bag_state->get_risk_map(avoid_all_items, risk_materials, layers_to_include);
          if(mongo_bag_state->risk_present)
          {
            Logger::Instance()->Warn("Potential risk regions detected in current bag state. Expanding risk map now.");
            risk_map_ptr = mongo_bag_state->expand_risk_map(risk_map_ptr, container->width, container->length, shadow_length,
                                                                 shadow_width, shadow_height, LFB_config_reader);
            risk_map_flag = true;
          }
          else
          {
            Logger::Instance()->Debug("No item damage risk detected in bag state. Continuing on with algorithm");
            risk_map_ptr = mongo_bag_state->get_zeros_map();
          }
      }
      else
      {
          Logger::Instance()->Debug("There are no item types at risk for this drop, or the bag is empty. Will not check bag state");
          risk_map_ptr = mongo_bag_state->get_zeros_map();
      }
    }
    catch (const std::exception & e)
    {
      Logger::Instance()->Error("Error from DropZoneSearcher when evaluating damage risk:\n{}", e.what());
    }
    catch (...)
    {
      Logger::Instance()->Error("Unspecified failure from DropZoneSearcher when evaluating damage risk ");
    }

    //Send item risk map to live viewer for visualization purposes
    if (this->drop_live_viewer != nullptr and risk_map_ptr != nullptr) {
      drop_live_viewer->update_image(drop_live_viewer->get_damage_risk_visualization(risk_map_ptr), ViewerImageType::LFB_Damage_Risk, this->PKID);
    }
  }

  /**
   *  Mongo bag state functionality ends here
   */
  Logger::Instance()->Debug("Retrieving local point cloud data now");
  std::shared_ptr<Eigen::Matrix3Xd> initial_local_data = point_cloud->get_data();
  Logger::Instance()->Debug("Retrieved local point cloud data");

  /**
   * ALTERNATIVE STREAMLINED CHECK FOR ITEM FITTING IN BAG BASED ON DROPGRID CLASS
   */
  float upper_depth_limit = float(details->remaining_platform - shadow_height) + this->acceptable_height_above_marker_surface; //assuming item lands perfectly vertical, need to be able to lower enough to make acceptable height above marker surface
  DropGrid drop_depth_grid = DropGrid(container->width, container->length, 22, 15); //TODO (SB): add grid squares to config if expected to change
  drop_depth_grid.populate_depth(point_cloud->get_data());
  bool drop_grid_fit_check_result = drop_depth_grid.check_whether_item_fits(shadow_width, shadow_length, upper_depth_limit);

  /**
  *  Check initial container point cloud for bag-too-full for any item condition --> should be sent directly to pickup
  */

  float avg_detected_depth = initial_local_data->rowwise().mean()[2];
  float bag_full_threshold_meters = LFB_config_reader->GetFloat("LFB_config", "bag_full_threshold_meters", 0.0);
  float threshold_comparison = avg_detected_depth - details->remaining_platform;
  Logger::Instance()->Debug("Bag too full check: avg_detected_depth: {}, remaining_platform: {}, threshold: {}, comparison: {}",
                           avg_detected_depth, details->remaining_platform, bag_full_threshold_meters, threshold_comparison);
  if(threshold_comparison > bag_full_threshold_meters)
  {
    Logger::Instance()->Error("Average depth analysis of bag indicates bag is too full for any more items!");
    if (drop_grid_fit_check_result) {
      Logger::Instance()->Error("Drop Grid Fit Check (Yes) disagrees with main algorithm (check 3); Cam: LFB");
    }
    this->success_code = DropTargetErrorCodes::NoViableTarget_BagIsFull;
    throw DropTargetError(DropTargetErrorCodes::NoViableTarget_BagIsFull,
                          std::string("Volume of bag is greater than the full check threshold. ") +
                          "{Threshold comparison = " + std::to_string(threshold_comparison) +
                          ", Average detected depth = " + std::to_string(avg_detected_depth) +
                          ", Remaining platform = " + std::to_string(details->remaining_platform) +
                          ", Bag full threshold in meters = " + std::to_string(bag_full_threshold_meters) +
                          "} so the calculations of [Threshold comparison = Average detected depth - Remaining platform]" +
                          " > Bag full threshold was true.");
  }

  //auto min_detected_Z = initial_local_data->rowwise().minCoeff()[2];

  /**
   *  Adjust depth detections based on white bag detections
   */
  //calculate minimum expected value for maxZ detections in LFB based on reported remaining_platform in request
  float platform_in_LFB_coords = 0.0F - (LFB_cavity_height - details->remaining_platform);

  bool bag_empty = (details->bag_item_count == 0);

  Logger::Instance()->Debug("This dispense does flip the X default to prefer the non-default side: {}", details->use_flipped_x_default);
  DropZoneSearcher::Max_Z_Points max_Z_points = adjust_depth_detections(RGB_matrix, point_cloud, details->item_mass, platform_in_LFB_coords, details->use_flipped_x_default,
    LFB_config_reader, true, true, bag_empty, false);

  Point3D max_Z_point = max_Z_points.overall;

  /**
   * Secondary check for bag too full, must be able to lower platform enough so tallest point in quadrant of bag of bag is low enough
   * TODO: names can be improved. These are all defined by x-y local bag coordinate quadrants, with "front" "back" etc. assuming bag is in nominal rotation state!
   */
  float allowed_item_overflow_pre_dispense = LFB_config_reader->GetFloat("LFB_config", "allowed_item_overflow_pre_dispense", 0.015); //meters TODO: make configurable value. Match VLSG recipe and post MaxZ check?
  float remaining_platform_adjusted = std::max(details->remaining_platform, 0.0F); //in case invalid negative values were provided from LFR

  bool front_left_no_viable_targets = std::max(max_Z_points.outer_front_left.z, max_Z_points.front_left.z) > (remaining_platform_adjusted + allowed_item_overflow_pre_dispense);
  bool front_right_no_viable_targets = std::max(max_Z_points.outer_front_right.z, max_Z_points.front_right.z) > (remaining_platform_adjusted + allowed_item_overflow_pre_dispense);

  bool back_left_no_viable_targets = std::max(max_Z_points.outer_back_left.z, max_Z_points.back_left.z) > (remaining_platform_adjusted + allowed_item_overflow_pre_dispense);
  bool back_right_no_viable_targets = std::max(max_Z_points.outer_back_right.z, max_Z_points.back_right.z) > (remaining_platform_adjusted + allowed_item_overflow_pre_dispense);

  bool front_no_viable_targets = (front_left_no_viable_targets && front_right_no_viable_targets) ||
      ( (front_left_no_viable_targets || front_right_no_viable_targets) && (details->item_width > LFB_bag_width/2.0) ); // todo change to tongue witdth not item width

  bool back_no_viable_targets =  (back_left_no_viable_targets && back_right_no_viable_targets) ||
      ( (back_left_no_viable_targets || back_right_no_viable_targets) && (details->item_width > LFB_bag_width/2.0) ); // TODO change to tongue width not item width

  if (front_no_viable_targets && back_no_viable_targets)
  {
    Logger::Instance()->Error("All quadrants of bag could result in item - dispense conveyor collision. Bag too full");
    if (drop_grid_fit_check_result) Logger::Instance()->Error("Drop Grid Fit Check (Yes) disagrees with main algorithm (check 4); Cam: LFB");
    this->success_code = DropTargetErrorCodes::NoViableTarget_BagIsFull;
    throw DropTargetError(DropTargetErrorCodes::NoViableTarget_BagIsFull,
                          "All quadrants of bag could result in item - dispense conveyor collision");
  }
  else
  {
    if (front_left_no_viable_targets) Logger::Instance()->Warn("Front Left of LFR could result in item - dispense conveyor collision. Will not consider targets there.");
    if (front_right_no_viable_targets) Logger::Instance()->Warn("Front Right of LFR could result in item - dispense conveyor collision. Will not consider targets there.");
    if (back_left_no_viable_targets) Logger::Instance()->Warn("Back Left of LFR could result in item - dispense conveyor collision. Will not consider targets there.");
    if (back_right_no_viable_targets) Logger::Instance()->Warn("Back Right of LFR could result in item - dispense conveyor collision. Will not consider targets there.");
  }

  /**
   * Bag empty handling
   */
  // if bag is empty: choose deterministic target of LFB. Otherwise, continue with target algorithm
  // note: this function is placed here after the adjust_depth_detections function so there are no missing visualizations and to allow for sanity checks that the bag is really empty.
  if (bag_empty and this->success_code != DropTargetErrorCodes::EmptyBagNotEmpty)
  {
    std::shared_ptr<Point3D> XYZ_result = get_empty_bag_target(details, LFB_config_reader, shadow_length, shadow_width, LFB_cavity_height);

    std::shared_ptr<cv::Mat> target_image = visualize_target(XYZ_result, container, shadow_length, shadow_width, RGB_matrix, 1, 3);
    if (this->visualize == 1) session_visualizer8->display_image(target_image);
    //handle visualizations for the empty bag case
    if (this->drop_live_viewer != nullptr)
    {
      this->drop_live_viewer->update_image(target_image, ViewerImageType::LFB_Target, this->PKID);
      if (mongo_bag_state != nullptr)
      {
        std::shared_ptr<cv::Mat> risk_map_ptr = mongo_bag_state->get_zeros_map();
        drop_live_viewer->update_image(drop_live_viewer->get_damage_risk_visualization(risk_map_ptr), ViewerImageType::LFB_Damage_Risk, this->PKID);
      }
    }
    Logger::Instance()->Info("Returning standard empty-bag target. Doesn't need to take into account rotated status of bot");
    return std::make_shared<DropResult>(XYZ_result, XYZ_result->z, false, bot_is_rotated,
      false, details->request_id, this->success_code, this->error_description);
  }

  // TODO: this conversion to pixel_depth is only necessary because the new_point_cloud function in the TranslatedPointCloud.cpp causes the new matrix to reference the same inner matrix as before, rather than creating a copy.
  // TODO: this issue was resolved by using the new_point_cloud function as available in depth_pixel_point_cloud.cpp which DOES build a copy of the inner matrix (important for average + variance calculations later in algorithm)
  Logger::Instance()->Trace("temporary debug: get pixel depth data");
  std::shared_ptr<std::vector<std::shared_ptr<std::pair<std::shared_ptr<cv::Point2f>, float>>>> pixel_depth_data = point_cloud->as_pixel_cloud()->get_data_with_depth();
  Logger::Instance()->Trace("temporary debug: get filtered point cloud");
  std::shared_ptr<LocalPointCloud> filtered_point_cloud = point_cloud->as_pixel_cloud()->new_point_cloud(pixel_depth_data)->as_local_cloud();

  if(this->visualize) session_visualizer4->display_image(session_visualizer4->display_points_with_depth_coloring(point_cloud));


  /**
   *   apply first filter on candidate targets based on item dimensions, container, and point cloud
   */
  std::shared_ptr<LocalPointCloud> valid_drop_centers = apply_box_limits(container, point_cloud, shadow_length, shadow_width, shadow_height,
                                                                    details->limit_left, details->limit_right, details->limit_front,
                                                                    details->limit_back, details->remaining_platform, LFB_width,
                                                                    LFB_length, container_width, container_length,
                                                                    front_edge_target_offset, port_edge_target_offset, bot_is_rotated);

  if(this->visualize) session_visualizer5->display_image(session_visualizer5->display_points_with_depth_coloring(valid_drop_centers));
  std::shared_ptr<Eigen::Matrix3Xd> valid_drop_center_data = valid_drop_centers->get_data(); //gets the inner matrix points in local coordinate system
  Logger::Instance()->Debug("Number of drop target candidates remaining after apply filter: {} out of original {}", valid_drop_center_data->cols(), original_candidate_count);
  if(valid_drop_center_data->cols() < 1)
  {
    Logger::Instance()->Error("No Viable Dispense Candidates; Cam: LFB");
    if (this->drop_live_viewer != nullptr) this->drop_live_viewer->update_image(this->session->get_color_mat(), ViewerImageType::LFB_Target, this->PKID);
    if (drop_grid_fit_check_result) Logger::Instance()->Error("Drop Grid Fit Check (Yes) disagrees with main algorithm (check 1); Cam: LFB");
    this->success_code = DropTargetErrorCodes::NoViableTarget_NoSpaceForItem;
    throw DropTargetError(DropTargetErrorCodes::NoViableTarget_NoSpaceForItem,
                          "No remaining drop target candidates after applying the first filter based on item dimensions, container, and point cloud");
  }

  //apply damage risk filter on candidate targets
  if(risk_map_flag)
  {
    valid_drop_center_data = mongo_bag_state->apply_risk_filter(container->width, container->length, valid_drop_center_data, *risk_map_ptr);
    Logger::Instance()->Debug("Number of drop target candidates remaining after damage risk filter: {} out of original {}", valid_drop_center_data->cols(), original_candidate_count);
    if(valid_drop_center_data->cols() < 1)
    {
      //TODO wtf why are we throwing for error code?!
      Logger::Instance()->Error("No Viable Drop Candidates, Damage Risk; Cam: LFB");
      if (this->drop_live_viewer != nullptr) this->drop_live_viewer->update_image(this->session->get_color_mat(), ViewerImageType::LFB_Target, this->PKID);
      mongo_bag_state->num_damage_rejections += 1;
      int max_allowable_damage_rejections = 2; //TODO: consider making this configurable
      if(mongo_bag_state->num_damage_rejections >= max_allowable_damage_rejections)
      {
        Logger::Instance()->Warn("Bag Reached Max Num Damage Rejections: {}; Cam: LFB", max_allowable_damage_rejections);
        this->success_code = DropTargetErrorCodes::NoViableTarget_DamageRisk_PickupRequired;
        throw DropTargetError(DropTargetErrorCodes::NoViableTarget_DamageRisk_PickupRequired,
                              std::string("No remaining drop target candidates after applying the second filter based on ") +
                              "damage risk, and the bag has been rejected the max number of times: " + std::to_string(max_allowable_damage_rejections));
      }
      this->success_code = DropTargetErrorCodes::NoViableTarget_DamageRisk;
      throw DropTargetError(DropTargetErrorCodes::NoViableTarget_DamageRisk,
                            std::string("No remaining drop target candidates after applying the second filter based on ") +
                            "damage risk. The bag has been rejected " + std::to_string(mongo_bag_state->num_damage_rejections) +
                            " times, and the max number of rejections is " + std::to_string(max_allowable_damage_rejections));
    }
  }

  /**
   *  only consider a subset of the remaining valid candidates to save processing time //TODO: this can be improved, consider using function similar to apply_risk_filter above
   *  //TODO: improve general window searching algorithm, thus making this filtering irrelevant
   */

  // Can't we just do this with loop step size instead
  int reduction_modulo_value = LFB_config_reader->GetInteger("LFB_config", "only_consider_every_Xth_target_candidate", 1);
  if(valid_drop_center_data->cols() < 100 || reduction_modulo_value == 1)
  {
    Logger::Instance()->Debug("Valid target candidate count is already under 100 in quantity or modulo config set to 1, will not remove further candidates");
  }
  else
  {
    int insert_index = 0;
    std::shared_ptr<Eigen::Matrix3Xd> filtered_valid_drop_candidates = std::shared_ptr<Eigen::Matrix3Xd>(new Eigen::Matrix3Xd(3, valid_drop_center_data->cols()));

    for(int index=0; index < valid_drop_center_data->cols(); index++)
    {
      if(index % reduction_modulo_value != 0) continue; //cut out candidate points with certain X coordinate mm values

      (*filtered_valid_drop_candidates)(0,insert_index) = valid_drop_center_data->col(index).x();
      (*filtered_valid_drop_candidates)(1,insert_index) = valid_drop_center_data->col(index).y();
      (*filtered_valid_drop_candidates)(2,insert_index) = valid_drop_center_data->col(index).z();
      insert_index++;
    }
    int remaining_points = insert_index;
    Logger::Instance()->Debug("Number of remaining candidate points after reduction is: {}", remaining_points);
    filtered_valid_drop_candidates->conservativeResize(3, remaining_points); //get rid of unpopulated values

    valid_drop_center_data = filtered_valid_drop_candidates;
  }

  /**
   * Visualize and find best target from the valid candidates
   */
  if(this->visualize)
  {
    //TODO: this is terribly inefficient, visualizations need refactor and shouldn't require a new PointCloud. But not executed during production so ok for now.
    std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> temp_point_cloud =  container->get_point_cloud(false);
    std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> remaining_candidate_point_cloud;
    remaining_candidate_point_cloud = temp_point_cloud->as_local_cloud()->new_point_cloud(valid_drop_center_data);
    session_visualizer9->display_image(session_visualizer9->display_points_with_depth_coloring(remaining_candidate_point_cloud));
  }

  struct Target_Region best_target_region {}; //initialize valid entry to false, not populated w/ data yet
  struct Target_Region current_target_region {}; //initialize valid entry to false, not populated w/ data yet

  Logger::Instance()->Trace("Cycle through all dispense candidates and select the best one");
  bool candidate_found = false;
  Logger::Instance()->Info("Algorithm proceeding to find best candidate, w/ valid candidate matrix of size: {}", valid_drop_center_data->cols());
  int candidates_that_require_rotation = 0;
  for(int i = 0; i < valid_drop_center_data->cols(); i++)
  {
    Point3D current_point = valid_drop_center_data->col(i);

    //TODO: these checks can potentially be applied earlier during the apply_box_limit function but would require redesign of that method
    //Applies a series of checks (potential item collisions w/ dispense conveyors, bot travel limits, dispense conveyor reach) to
    //See if nominal bot rotation will allow for dispense to the given candidate target or if pirouette is needed
    // TODO: default is to always dispense in nominal orientation if possible. This is potentially sub-optimal
    bool must_rotate_from_nominal_to_reach_candidate = false;
    if( (bot_is_rotated && !this->LFB_rotation_allowed) ||
        front_no_viable_targets ||
        (front_left_no_viable_targets && current_point.x < 0) ||
        (front_right_no_viable_targets && current_point.x > 0) ||
        (current_point.y < this->rotation_limit_line) ||
        (current_point.x > (LFB_width/2 - ((float)details->limit_right)/1000) ) ||
        (current_point.x < (-LFB_width/2 + ((float)details->limit_left)/1000) ))
      must_rotate_from_nominal_to_reach_candidate = true; //candidate is not viable when in nominal orientation

    if(must_rotate_from_nominal_to_reach_candidate) {
      candidates_that_require_rotation++;
      if ( (!bot_is_rotated && !this->LFB_rotation_allowed) ||
           back_no_viable_targets ||
           (back_left_no_viable_targets && current_point.x < 0) ||
           (back_right_no_viable_targets && current_point.x > 0) ||
           (current_point.y > (-1.0 * this->rotation_limit_line)) ||
           (-1.0 * current_point.x > (LFB_width / 2 - ((float)details->limit_right) / 1000)) ||
           (-1.0 * current_point.x < (-LFB_width / 2 + ((float)details->limit_left) / 1000)))
        continue; //candidate is not viable for dispense when in rotated 180 degree state from nominal orientation, cannot be reached
    }

    current_target_region = analyze_target_region(shadow_length, shadow_width, current_point, filtered_point_cloud, max_Z_point); //Todo: test this function more thoroughly
    current_point.z = current_target_region.z; // update z to match the target region z

    // store whether the current candidate requires 180 degree pirouette rotation (RELATIVE TO NOMINAL BOT ORIENTATION)
    current_target_region.rotation_required = must_rotate_from_nominal_to_reach_candidate;

    //TODO: add conditions for early failing a candidate if it will not be optimal without needing to do interference region check (take from compare candidates).
    current_target_region.interference_region = define_interference_region(details->item_length, shadow_width, details->item_height,
                                                                           current_point, must_rotate_from_nominal_to_reach_candidate);

    std::shared_ptr<Target_Region> current_target_ptr = std::make_shared<Target_Region>(current_target_region);
    check_interference(current_target_ptr, filtered_point_cloud);

    // Check if candidate point will result in item sticking out too far above bag (collision and non-collision cases). if so, continue on to next candidate
    float dispensed_item_expected_max_Z_collision = -1;
    if(current_target_ptr->interference_detected == true) //if current point involves interference, check that item won't go above allowable Z
    {
      //Note: this calculation currently makes use of the average Z in the interference region, not the max Z in that region
      dispensed_item_expected_max_Z_collision = current_target_region.interference_average_z + details->item_height;
      Logger::Instance()->Trace("Remaining Platform is: {:.2f}, max_Z is: {:.2f}, collision_Z is: {:.2f}, non-collision_z is: {:.2f}, item height is: {:.2f}, shadow height is: {:.2f}",
                                details->remaining_platform, max_Z_point.z, current_target_region.z, best_target_region.z, details->item_height, shadow_height);
      if (dispensed_item_expected_max_Z_collision > (details->remaining_platform + this->acceptable_height_above_marker_surface))
      {
        Logger::Instance()->Trace("Removed candidate from consideration due to potential Z height after dispense. Collision case");
        continue;
      }
    }
    float dispensed_item_expected_max_Z_no_collision = current_target_region.z + shadow_height;
    if (dispensed_item_expected_max_Z_no_collision > (details->remaining_platform + this->acceptable_height_above_marker_surface))
    {
      Logger::Instance()->Trace("Removed candidate from consideration due to potential Z height after dispense. No-collision case.");
      continue;
    }

    //compare current candidate to best candidate up to this point, as long as the interference state would not worsen with the new candidate
    bool update_flag = false;
    if(candidate_found == false) //if this is first acceptable candidate, set update flag to be true. Will be set to best candidate
    {
      update_flag = true;
    }
    else
    {
      update_flag = compare_candidates(std::make_shared<Target_Region>(best_target_region), current_target_ptr);
    }

    if (update_flag == true)
    {
      candidate_found = true;
      best_target_region = *current_target_ptr;
      Logger::Instance()->Debug("New Best Candidate. X: {:.3f}, Y: {:.3f}, Z: {:.3f}, Average: {:.3f}, Variance: {:.1f}, Max: {:.3f}, "
                                "Min: {:.3f}, Range: {:.3f}, MinDist: {:.3f}, Rotate: {}, Interf.: {}, Interf.max {:.3f}, Interf.avg {:.3f},"
                                "maxZ_collision: {}, maxZ_no_collision: {}",
                                (best_target_region.x),
                                best_target_region.y,
                                best_target_region.z,
                                best_target_region.average_depth,
                                best_target_region.variance_depth,
                                best_target_region.max_Z,
                                best_target_region.min_Z,
                                best_target_region.range_depth,
                                best_target_region.distance_to_max_LFB_Z,
                                best_target_region.rotation_required, //RELATIVE TO NOMINAL BOT ORIENTATION, NOT RELATIVE TO THE CURRENT STATE OF BOT
                                best_target_region.interference_detected,
                                best_target_region.interference_max_z,
                                best_target_region.interference_average_z,
                                dispensed_item_expected_max_Z_collision,
                                dispensed_item_expected_max_Z_no_collision);
    }
  }
  Logger::Instance()->Debug("Finished cycling through all region options");
  Logger::Instance()->Info("Target requires 180 deg bot rotation relative to nominal bot orientation: {}. "
    "With {} rotation required potential targets.", best_target_region.rotation_required, candidates_that_require_rotation);

  if(!candidate_found)
  {
    Logger::Instance()->Error("No Valid Dispense Candidates; Cam: LFB");
    if (drop_grid_fit_check_result) Logger::Instance()->Error("Drop Grid Fit Check (Yes) disagrees with main algorithm (check 2); Cam: LFB");
    if (this->drop_live_viewer != nullptr) this->drop_live_viewer->update_image(this->session->get_color_mat(), ViewerImageType::LFB_Target, this->PKID);
    this->success_code = DropTargetErrorCodes::NoViableTarget_NoSpaceForItem;
    throw DropTargetError(DropTargetErrorCodes::NoViableTarget_NoSpaceForItem,
                          std::string("No valid drop target candidates remain after filtering on potential dispense arm collisions, ") +
                          "bot travel limits, dispense conveyor reach, etc. Pirouettes are allowed for this dispense: " + std::to_string(this->LFB_rotation_allowed) +
                          ", and this LFB has already rotated for this dispense: " + std::to_string(bot_is_rotated));
  }

  if (!drop_grid_fit_check_result) Logger::Instance()->Error("Drop Grid Fit Check (No) disagrees with main algorithm; Cam: LFB");
  std::shared_ptr<Point3D> XYZ_result = std::make_shared<Point3D>(best_target_region.x, best_target_region.y, best_target_region.z);

  //modify target Z if potential swing collision is expected
  if(best_target_region.interference_detected)
  {
    float modified_target_z = best_target_region.interference_max_z; // (best_target_region.interference_average_z + best_target_region.interference_max_z)/2.0;
    if(modified_target_z > XYZ_result->z)
    {
      Logger::Instance()->Info("Updating drop target Z to {} to account for swing collision interference", modified_target_z);
      XYZ_result->z = modified_target_z;
    }
    else
    {
      Logger::Instance()->Warn("Even though swing collision interference detected, will not update target Z because not higher than current value");
    }
  }

  if(this->visualize == 1 || this->drop_live_viewer != nullptr)
  {
    std::shared_ptr<cv::Mat> output_image;
    if (this->visualize_interference_zone) // add interference zone to visualization 6
    {
      std::shared_ptr<cv::Mat> target_image = visualize_target(std::make_shared<Point3D>(best_target_region.interference_region.center), container,
                                                               best_target_region.interference_region.length,
                                                               best_target_region.interference_region.width, RGB_matrix, 2, 2);

      output_image = visualize_target(XYZ_result, container, shadow_length, shadow_width, target_image, 1, 3);
    }
    else // visualization only includes the target region
    {
      output_image = visualize_target(XYZ_result, container, shadow_length, shadow_width, RGB_matrix, 1, 3);
    }

    if (this->visualize == 1) session_visualizer8->display_image(output_image);
    if (this->drop_live_viewer != nullptr) this->drop_live_viewer->update_image(output_image, ViewerImageType::LFB_Target, this->PKID);
  }

  //provided x and y target to VLSG must always be in the VLS coordinate frame.
  if(best_target_region.rotation_required) //recall this means that dispense will happen with bot rotated 180 degrees FROM NOMINAL
  {
    Logger::Instance()->Info("Modifying sign of target X and Y to account for bot rotation from nominal for dispense");
    XYZ_result->x = -1*XYZ_result->x;
    XYZ_result->y = -1*XYZ_result->y;
  }

  //confirm that resulting target lies within acceptable bounds
  // TODO: @Amber I don't want to blindly mess with conditionals at this point, but
  //  I don't think that we want to use the bitwise or here, and probably want to add in some parenthesis
  if (abs(XYZ_result->x) > LFB_bag_width/2.0 | abs(XYZ_result->y) > LFB_bag_length/2.0 | abs(XYZ_result->z) > (0.45) | abs(max_Z_point.z) > (0.45))
  {
    Logger::Instance()->Error("Error: Target or MaxZ in bag not within expected bounds! X: {}, Y: {}, Z: {}, MaxZ: {}", XYZ_result->x, XYZ_result->y, XYZ_result->z, max_Z_point.z);
    this->success_code = DropTargetErrorCodes::AlgorithmFail_TargetOutOfBounds;
    throw DropTargetError(DropTargetErrorCodes::AlgorithmFail_TargetOutOfBounds,
                          "TargetZ or MaxZ is not within expected bounds. Values: { X = " + std::to_string(XYZ_result->x) +
                          ", Y = " + std::to_string(XYZ_result->y) +
                          ", Z = " + std::to_string(XYZ_result->z) +
                          ", MaxZ = " + std::to_string(max_Z_point.z) + " }");
  }

  /**
   * VLSG code will attempt to adjust platform pre-dispense so that the max_Z_result lines up with marker surface
   * This calculation ensures that the dispense-side highest point will align with markers, unless an item on the other side
   * would go more than 100mm above the marker surface, and in that case we limit it to 100mm
   * TODO: in the future, we can apply this more narrowly to just a region around the dispense target, especially when bot size increases
  */
  float item_protrusion_detection_threshold = LFB_config_reader->GetFloat("LFB_config", "item_protrusion_detection_threshold", 0.005);
  float max_Z_on_dispense_side = get_max_z_on_dispense_side(max_Z_points, best_target_region.rotation_required, item_protrusion_detection_threshold);
  float max_Z_alternate_value = max_Z_points.overall.z - max_acceptable_Z_above_marker_surface;
  float max_Z_result = std::max(max_Z_on_dispense_side, max_Z_alternate_value);

  bool tell_VLSG_to_rotate_bot_from_current_state = best_target_region.rotation_required != bot_is_rotated;

  Logger::Instance()->Debug("Max_Z on dispense side is: {}, alternate max_Z is: {}", max_Z_on_dispense_side, max_Z_alternate_value);
  Logger::Instance()->Debug("Drop Target Algorithm finished with success code: {}", this->success_code);
  return std::make_shared<DropResult>(XYZ_result, max_Z_result, tell_VLSG_to_rotate_bot_from_current_state, bot_is_rotated,
                                                    best_target_region.interference_detected, details->request_id, this->success_code, this->error_description);
}

float DropZoneSearcher::get_max_z_on_dispense_side(DropZoneSearcher::Max_Z_Points max_Z_points, bool rotation_required, float item_protrusion_detection_threshold)
{
    // nominal back side
    if (rotation_required) {
        // if the outer bag's max Z is greater than marker surface then use that instead
        return (max_Z_points.outer_back.z > item_protrusion_detection_threshold) ? max_Z_points.outer_back.z : max_Z_points.back.z;
        // nominal front side
    } else {
        // if the outer bag's max Z is greater than marker surface then use that instead
        return (max_Z_points.outer_front.z > item_protrusion_detection_threshold) ? max_Z_points.outer_front.z : max_Z_points.front.z;
    }
}

std::shared_ptr<fulfil::dispense::commands::PostLFRResponse> DropZoneSearcher::find_max_Z(std::shared_ptr<MarkerDetectorContainer> container, std::shared_ptr<std::string> request_id,
                                                               std::shared_ptr<INIReader> LFB_config_reader, std::shared_ptr<mongo::MongoBagState> mongo_bag_state,
                                                               std::shared_ptr<nlohmann::json> request_json, std::shared_ptr<std::vector<std::string>> cached_info)
{
  Logger::Instance()->Trace("find_drop_zone_center called in drop_zone_searcher");

  float LFB_cavity_height = LFB_config_reader->GetFloat("LFB_config", "LFB_cavity_height", -1);
  float remaining_platform = (*request_json)["Remaining_Platform"].get<float>()/1000; //remaining_platform in meters
  float should_search_right_to_left = request_json->value("Flip_X_Default", false);

    Logger::Instance()->Debug("Check MaxZ Initiated");

  Logger::Instance()->Trace("Get RGB image for use in algorithm and visualizations");
  std::shared_ptr<cv::Mat> RGB_matrix = container->get_color_mat();
  session_visualizer1->wait_time = 0;
  if (this->visualize == 1) session_visualizer1->display_rgb_image(RGB_matrix);

  // Detect Aruco markers in RGB stream. The check that at least 4 markers were detected is in the detect_markers() function implementation

  if (this->visualize == 1)
  {
    std::shared_ptr<std::vector<std::shared_ptr<fulfil::depthcam::aruco::Marker>>> markers = container->get_markers();
    session_visualizer2->draw_detected_markers(container->marker_detector->dictionary,
                                               markers,
                                               container->region_max_x,
                                               container->region_min_x,
                                               container->region_max_y,
                                               container->region_min_y);
  }


  std::shared_ptr<LocalPointCloud> point_cloud = container->get_point_cloud(false)->as_local_cloud();

  if (this->visualize == 1) session_visualizer3->display_image(session_visualizer3->display_points_with_depth_coloring(point_cloud));

  Logger::Instance()->Trace("Number of point cloud points before filter (out of a possible 92x160 = 14,720): {}",
                            point_cloud->get_data()->cols());

  std::shared_ptr<Eigen::Matrix3Xd> initial_local_data = point_cloud->get_data();


  //calculate minimum expected value for maxZ detections in LFB based on reported remaining_platform in request
  float platform_in_LFB_coords = 0.0 - (LFB_cavity_height - remaining_platform);
  if (platform_in_LFB_coords < -0.4 or platform_in_LFB_coords > -0.01)
  {
    Logger::Instance()->Error("Error with minimum max depth. Reported as {} with remaining_platform as: {}", platform_in_LFB_coords, remaining_platform);
  }
  Logger::Instance()->Debug("Minimum max depth to be used for maxZ calculation in adjustment is: {}", platform_in_LFB_coords);

  //this function adjusts depths of the white parts of the bag and returns the max_Z depth detection of non-white bag areas

  DropZoneSearcher::Max_Z_Points max_Z_points = adjust_depth_detections(RGB_matrix, point_cloud, 1000, platform_in_LFB_coords, should_search_right_to_left, LFB_config_reader, true, false, false, false);
  Point3D max_detected_Z_point = max_Z_points.overall;

  /**
  * Secondary check for bag too full, must be able to lower platform enough so tallest point in half of bag is at or below marker surface
  */
  //TODO: make configurable value. This value must be positive because current algo searches over bot markers as well, so expect minimum MaxZ values of ~0.0 +/- 5mm
  float allowed_item_overflow_post_dispense_check = LFB_config_reader->GetFloat("LFB_config", "allowed_item_overflow_post_dispense_check", 0.015);
  float remaining_platform_adjusted = std::max(remaining_platform, float(0.0)); //in case invalid negative values were provided from LFR
  bool front_side_no_viable_targets = max_Z_points.front_left.z > (remaining_platform_adjusted + allowed_item_overflow_post_dispense_check) &&
      max_Z_points.front_right.z > (remaining_platform_adjusted + allowed_item_overflow_post_dispense_check);
  bool back_side_no_viable_targets = max_Z_points.back_left.z > (remaining_platform_adjusted + allowed_item_overflow_post_dispense_check) &&
      max_Z_points.back_right.z > (remaining_platform_adjusted + allowed_item_overflow_post_dispense_check);


  int bag_full_result = -1;
  if (mongo_bag_state != nullptr)
  {
    if(mongo_bag_state->packing_state != nullptr)
    {
      bag_full_result = mongo_bag_state->packing_state->update_detected_volume(point_cloud->get_data(), remaining_platform);
      Logger::Instance()->Debug("Bag full % is {}", bag_full_result);
      std::string message = "Bag Full: " + std::to_string(bag_full_result) + "%";
      if(cached_info != nullptr) cached_info->push_back(message);
    }
    else
    {
      Logger::Instance()->Error("Could not update packing state because packing_state is nullptr!");
    }
  }
  else
  {
    Logger::Instance()->Error("Could not update packing state because mongo_bag_state is nullptr!");
  }

  Logger::Instance()->Debug("Post-Dispense Analysis, max_Z is: {} mm", int(max_detected_Z_point.z * 1000));

  if (this->visualize == 1)
  {
    session_visualizer4->wait_time = 0;
    session_visualizer4->display_image(session_visualizer4->display_points_with_depth_coloring(point_cloud));
  }

  float max_Z_result = max_detected_Z_point.z;
  if (front_side_no_viable_targets && back_side_no_viable_targets)
  {
    Logger::Instance()->Warn("Both sides of LFR could result in future item - dispense conveyor collision. Will send bag to pickup");
    max_Z_result = 1.0; //returning such a large value will automatically lead to bag sent to pickup. //TODO: could handle more elegantly w/ additional output
  }

  return std::make_unique<commands::PostLFRResponse>(request_id, 0, max_Z_result, -1, bag_full_result);
}




