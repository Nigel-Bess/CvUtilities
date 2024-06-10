//
// Created by nkaffine on 12/9/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_SRC_DROP_ZONE_SEARCHER_H_
#define FULFIL_DISPENSE_SRC_DROP_ZONE_SEARCHER_H_
#include <memory>
#include <vector>
#include <Fulfil.CPPUtils/point_3d.h>
#include <Fulfil.DepthCam/aruco.h>
#include <Fulfil.Dispense/drop/drop_result.h>
#include<Fulfil.DepthCam/visualization.h>
#include <Fulfil.Dispense/visualization/live_viewer.h>
#include <Fulfil.Dispense/commands/dispense_request.h>
#include <Fulfil.Dispense/commands/dispense_response.h>
#include <Fulfil.Dispense/mongo/mongo_bag_state.h>
#include <Fulfil.Dispense/commands/post_drop/post_LFR_response.h>


namespace fulfil::dispense::drop
{
/**
 * The purpose of this class to encapsulate all the logic
 * behind determining the opimtal drop center for a drop
 * zone of given length and width in the provided container.
 */
class DropZoneSearcher
{
 private:

  /**
  *  Checks for correct inputs to algorithm
  **/
  void check_inputs(float shadow_length,
                    float shadow_width,
                    float shadow_height,
                    std::shared_ptr<LfbVisionConfiguration> lfb_vision_config,
                    std::shared_ptr<fulfil::dispense::commands::DropTargetDetails> details);


  /**
  *  Checks that enough markers were detected and that the markers have valid depth data at center coordinates
  **/
  void check_markers(std::shared_ptr<std::vector<std::shared_ptr<fulfil::depthcam::aruco::Marker>>> markers, std::shared_ptr<fulfil::depthcam::aruco::MarkerDetectorContainer> container);

  /**
  *  Applies filter on point cloud data so only data within confines of LFB bag (with some buffer) are considered
  **/
  std::shared_ptr<fulfil::depthcam::pointcloud::LocalPointCloud> apply_box_limits(
          std::shared_ptr<fulfil::depthcam::aruco::MarkerDetectorContainer> container,
          std::shared_ptr<fulfil::depthcam::pointcloud::LocalPointCloud> point_cloud,
          float shadow_length, float shadow_width, float shadow_height, int limit_left, int limit_right, int limit_front,
          int limit_back, float remaining_platform, float LFB_width, float LFB_length, float cavity_width, float cavity_length,
          float front_edge_target_offset, float port_edge_target_offset, int bot_is_rotated);


  struct Interference_Region
  {
    fulfil::utils::Point3D center{0,0,0};                 // center of interference search region, z matches z of associated target region
    float width{};                   // see diagram, this is along the x axis of the LFB bag
    float length{};                  // see diagram, this is along the y axis of the LFB bag
    Interference_Region() = default;
  };

  struct Target_Region
  {
    float average_depth{};
    float variance_depth{};
    float max_Z{};                    // note that positive Z is above the marker detector surface, negative Z is into the bag
    float min_Z{};
    float x{}, y{}, z{};                 // center of target region
    float range_depth{};
    float distance_to_max_LFB_Z{};   // distance to max Z value in the entire LFB (tallest item), not just within the target region
    float interference_max_z{};       // max z in LFB local coordinates of points detected in interference zone
    float interference_average_z {};       // average z in LFB local coordinates of points detected in interference zone
    bool rotation_required{};       // indicates whether LFB rotation will be required to reach this candidate point
    bool interference_detected{};    // true if potential interference detected for this drop region
    Interference_Region interference_region{};
    // bleh should init all in one place
    Target_Region() = default;
  };

  /**
   * Struct for storing the points for maximum depth across different regions of the LFR bag. See depth cam diagrams
   * for definition of "front" and "back" of bag.
   * "Front" has Y local bag coordinate that is > 0 ,
   * "Back" has Y local bag coordinate that is < 0
   * "Left" has X local bag coordinate that is < 0,
   * "Right" has X local bag coordinate that is > 0
   */
  struct Max_Z_Points
  {
    fulfil::utils::Point3D overall;
    fulfil::utils::Point3D front;
    fulfil::utils::Point3D back;
    fulfil::utils::Point3D front_left;
    fulfil::utils::Point3D front_right;
    fulfil::utils::Point3D back_left;
    fulfil::utils::Point3D back_right;
    fulfil::utils::Point3D outer_front_left;
    fulfil::utils::Point3D outer_front_right;
    fulfil::utils::Point3D outer_back_left;
    fulfil::utils::Point3D outer_back_right;
    fulfil::utils::Point3D outer_overall;
    fulfil::utils::Point3D outer_front;
    fulfil::utils::Point3D outer_back;
    int number_of_points_protruding;
  };

  /**
   * Analyzes target region candidate and populates Target Region struct with results
   * @param shadow_length of the drop shadow of the item being dropped.
   * @param shadow_width of the drop shadow of the item being dropped.
   * @param center of the drop zone
   * @param pointer to point cloud of points that are in the bag to be averaged.
   * @return
   */
  Target_Region analyze_target_region(float shadow_length, float shadow_width, fulfil::utils::Point3D center,
                                      std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud, fulfil::utils::Point3D max_Z_point);

  /**
   * Analyzes target region candidate and populates Target Region struct with results
   */

  Interference_Region define_interference_region(float item_length, float item_width, float item_height, fulfil::utils::Point3D center,
                                                 bool target_required_rotation_from_nominal);

  /**
  * Returns true if there is detected interference in the interference region
  */
  void check_interference(std::shared_ptr<Target_Region> target_region, std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud);

  /**
   * Returns true if the current candidate is better than the best candidate up to that point
   */
  bool compare_candidates(std::shared_ptr<Target_Region> best_target_region,
                          std::shared_ptr<Target_Region> current_target_region,
                          bool use_quadrant_preference_order,
                          std::shared_ptr<std::vector<std::string>> quadrant_preference_order);

  /**
   * Uses bag state info and current dispense item damage details to
   */
  void do_damage_analysis();

  /**
   *  Determines if bot is rotated from nominal
   *  Output: 0 = nominal (not rotated), 1 = rotated, -1 = could not determine
   */
  int check_bot_rotated(std::vector<std::shared_ptr<fulfil::depthcam::aruco::Marker>> markers, bool use_y_coordinates, int min_marker_count_for_validation);

  /**
   * Checks that the markers are in correct positions
   */
  void validate_marker_positions(bool nominal_bot_rotation, std::vector<std::shared_ptr<fulfil::depthcam::aruco::Marker>> markers,
                                 std::shared_ptr<LfbVisionConfiguration> lfb_vision_config);



  std::shared_ptr<fulfil::depthcam::Session> session;
  int debug;
  int visualize;
  int force_error;
  int success_code;
  //Global variables for affecting algorithm
  float shadow_buffer;
  int item_mass_threshold;
  int min_bag_filtering_threshold;
  float white_region_depth_adjust_from_min; //difference between lowest detection ("new_depth") and what white regions are modified to
  int empty_bag_threshold;



  float interference_depth_tolerance; // depth above target Z that counts as interference
  int num_interference_points_tolerance;

  float interference_region_length_factor;
  bool visualize_interference_zone;
  float max_acceptable_Z_above_marker_surface;
  float acceptable_height_above_marker_surface;

  float significant_variance_regression;
  float moderate_variance_regression;
  float moderate_variance_improvement;
  float significant_variance_improvement;

  float significant_depth_regression;
  float equivalent_depth;
  float moderate_depth_improvement;
  float significant_depth_improvement;
  float crazy_depth_improvement;
  float crazy_depth_regression;

  bool LFB_rotation_allowed = false;
  //limit beyond which candidate dispense centers require LFB rotation. In meters, LFB coordinate system Y coordinate
  //note: the directionality of the limit line depends on the rotation orientation of the box (see depth cam diagram slides for details)
  float rotation_limit_line = -1.0;

  std::shared_ptr<fulfil::dispense::visualization::LiveViewer> drop_live_viewer;

  std::shared_ptr<depthcam::visualization::SessionVisualizer> session_visualizer1;
  std::shared_ptr<depthcam::visualization::SessionVisualizer> session_visualizer2;
  std::shared_ptr<depthcam::visualization::SessionVisualizer> session_visualizer3;
  std::shared_ptr<depthcam::visualization::SessionVisualizer> session_visualizer4;
  std::shared_ptr<depthcam::visualization::SessionVisualizer> session_visualizer5;
  std::shared_ptr<depthcam::visualization::SessionVisualizer> session_visualizer6;
  std::shared_ptr<depthcam::visualization::SessionVisualizer> session_visualizer7;
  std::shared_ptr<depthcam::visualization::SessionVisualizer> session_visualizer8;
  std::shared_ptr<depthcam::visualization::SessionVisualizer> session_visualizer8andahalf;
  std::shared_ptr<depthcam::visualization::SessionVisualizer> session_visualizer9;
  std::shared_ptr<depthcam::visualization::SessionVisualizer> session_visualizer9andahalf;



public:
  /**
   * Calculates the optimal drop zone for the provided length and width (taking into
   * consideration obstructions on left and right side of the bags) in the provided
   * container.
   * @param shadow_length of the drop shadow of the item being dropped.
   * @param shadow_width of the drop shadow of the item being dropped.
   * @param clip_right the distance (in meters) to clip the right side of the bag because
   * it is not available for drops.
    * it is not available for drops.
   * @param container where the drop is occurring.
   * @return pointer to the center of the optimal drop zone of the given size in the container.
   */
  DropZoneSearcher(std::shared_ptr<fulfil::depthcam::Session> session,
     int visualize,
     int force_error,
     float shadow_buffer,
     int item_mass_threshold,
     std::shared_ptr<fulfil::dispense::visualization::LiveViewer> drop_live_viewer,
     int debug, int min_bag_filtering_threshold, float white_region_depth_adjust_from_min,
     int empty_bag_threshold,
     float interference_depth_tolerance, int num_interference_points_tolerance,
     float interference_region_length_factor,
     bool visualize_interference_zone, float max_acceptable_Z_above_marker_surface,
     float significant_variance_regression, float moderate_variance_regression,
     float moderate_variance_improvement, float significant_variance_improvement,
     float significant_depth_regression,
     float equivalent_depth, float moderate_depth_improvement, float significant_depth_improvement,
     float crazy_depth_regression, float crazy_depth_improvement);




  //the dispense ID, or "PrimaryKeyID" as it is called throughout the repo
  std::string PKID;
  /* Description of the error codes thrown during drop search algorithm */
  std::string error_description;

   /**
   *  Visualize resulting target for the dispense with a rectangular box of where the item's shadow should be
   *  Color:  1 = green, 2 = red, 3 = blue
   **/
  std::shared_ptr<cv::Mat> visualize_target(std::shared_ptr<fulfil::utils::Point3D> result,
          std::shared_ptr<fulfil::depthcam::aruco::MarkerDetectorContainer> container,
          float shadow_length, float shadow_width, std::shared_ptr<cv::Mat>, int color, int thickness);


  std::shared_ptr<DropResult> find_drop_zone_center(std::shared_ptr<fulfil::depthcam::aruco::MarkerDetectorContainer> container, std::shared_ptr<fulfil::dispense::commands::DropTargetDetails> details,
                                                    std::shared_ptr<LfbVisionConfiguration> lfb_vision_config, std::shared_ptr<fulfil::mongo::MongoBagState> mongo_bag_state, bool bot_has_already_rotated = false);


  std::shared_ptr<fulfil::utils::Point3D> get_empty_bag_target(std::shared_ptr<fulfil::dispense::commands::DropTargetDetails> details,
                                                std::shared_ptr<LfbVisionConfiguration> lfb_vision_config, float shadow_length, float shadow_width,
                                                float LFB_cavity_height);

    /**
   * Returns the value of the max Z in the LFB
   * @param max_Z_points the max Z points detected across the LFB
   * @param rotation_required whether the LFB will need to pirouette/rotate for this dispense
   * @param item_protrusion_detection_threshold amount in meters that the max Z must be detected above the marker plane (AKA the top of LFB)
   * @return max Z detected
   */
  std::shared_ptr<fulfil::utils::Point3D> get_max_z_from_max_points(DropZoneSearcher::Max_Z_Points max_Z_points, bool rotation_required, float item_protrusion_detection_threshold);


  std::shared_ptr<fulfil::dispense::commands::PostLFRResponse> find_max_Z(std::shared_ptr<fulfil::depthcam::aruco::MarkerDetectorContainer> container, std::shared_ptr<std::string> request_id,
                                               std::shared_ptr<LfbVisionConfiguration> lfb_vision_config, std::shared_ptr<fulfil::mongo::MongoBagState> mongo_bag_state,
                                               std::shared_ptr<nlohmann::json> request_json, std::shared_ptr<std::vector<std::string>> cached_info,
                                               std::shared_ptr<std::string> base_directory);

  /**
   *  Note: see marker_detector_container.h for notes on usage of extend_region_over_markers param. Currently it does nothing
   */
  std::shared_ptr<fulfil::depthcam::aruco::MarkerDetectorContainer> get_container(std::shared_ptr<LfbVisionConfiguration> lfb_vision_config,
                                                                                  std::shared_ptr<fulfil::depthcam::Session> session,
                                                                                  bool extend_region_over_markers);

  /**
   * Handles the detection of items on the ground during a dispense sequence, during the PostDropImage.
   */
  std::tuple<bool, bool, float> detect_item_on_ground_during_post_drop(std::string base_directory);

    /**
  *  Modifies the current container cavity local point cloud to treat detected parts of the white bag as if the detected depth were at the bottom of the cavity
  * @param   live_viewer_flag - this input was added to prevent saving filter visualization during the post-dispense call to adjust_depth_detections
  * @return See definition of Max_Z_Points struct above. Coordinates are provided in meter units, local bag coordinate system
  */
  Max_Z_Points adjust_depth_detections(std::shared_ptr<cv::Mat>, std::shared_ptr<fulfil::depthcam::pointcloud::LocalPointCloud> input_cloud,
                                  float item_mass, float minimum_max_depth, bool should_search_right_to_left, std::shared_ptr<LfbVisionConfiguration> lfb_vision_config, bool visualize_flag = true,
                                  bool live_viewer_flag = false, bool should_check_empty = false, bool force_adjustment = false);



};
} // namespace fulfil

#endif // FULFIL_DISPENSE_SRC_DROP_ZONE_SEARCHER_H_