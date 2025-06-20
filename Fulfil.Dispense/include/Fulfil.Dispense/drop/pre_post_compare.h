//
// Created by nkaffine on 12/9/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_SRC_PRE_POST_COMPARE_H_
#define FULFIL_DISPENSE_SRC_PRE_POST_COMPARE_H_
#include <memory>
#include <vector>
#include <Fulfil.CPPUtils/point_3d.h>
#include <Fulfil.DepthCam/aruco.h>
#include<Fulfil.DepthCam/visualization.h>
#include <Fulfil.Dispense/drop/drop_result.h>
#include <Fulfil.Dispense/commands/dispense_request.h>
#include <Fulfil.Dispense/commands/dispense_response.h>
#include <Fulfil.Dispense/visualization/live_viewer.h>

namespace fulfil::dispense::drop
{
/**
 * The purpose of this class to encapsulate all the logic
 * behind determining the opimtal drop center for a drop
 * zone of given length and width in the provided container.
 */
class PrePostCompare
{
 private:

  int visualize;
  std::shared_ptr<DropZoneSearcher> drop_zone_searcher;

  std::shared_ptr<fulfil::depthcam::visualization::SessionVisualizer> session_visualizer10;
  std::shared_ptr<fulfil::depthcam::visualization::SessionVisualizer> session_visualizer11;
  std::shared_ptr<fulfil::depthcam::visualization::SessionVisualizer> session_visualizer12;
  std::shared_ptr<fulfil::depthcam::visualization::SessionVisualizer> session_visualizer13;
  std::shared_ptr<fulfil::depthcam::visualization::SessionVisualizer> session_visualizer14;
  std::shared_ptr<fulfil::depthcam::visualization::SessionVisualizer> session_visualizer15;
  std::shared_ptr<fulfil::depthcam::visualization::SessionVisualizer> session_visualizer16;
  std::shared_ptr<fulfil::depthcam::visualization::SessionVisualizer> session_visualizer17;
  std::shared_ptr<fulfil::depthcam::visualization::SessionVisualizer> session_visualizer18;
  std::shared_ptr<fulfil::depthcam::visualization::SessionVisualizer> session_visualizer19;
  std::shared_ptr<fulfil::depthcam::visualization::SessionVisualizer> session_visualizer20;
  std::shared_ptr<fulfil::depthcam::visualization::SessionVisualizer> session_visualizer21;
  std::shared_ptr<fulfil::depthcam::visualization::SessionVisualizer> session_visualizer22;
  std::shared_ptr<fulfil::depthcam::visualization::SessionVisualizer> session_visualizer23;
  std::shared_ptr<fulfil::depthcam::visualization::SessionVisualizer> session_visualizer24;
  std::shared_ptr<fulfil::depthcam::visualization::SessionVisualizer> session_visualizer25;
  std::shared_ptr<fulfil::depthcam::visualization::SessionVisualizer> session_visualizer26;
  std::shared_ptr<fulfil::depthcam::visualization::SessionVisualizer> session_visualizer27;
  std::shared_ptr<fulfil::depthcam::visualization::SessionVisualizer> session_visualizer28;

  float container_length;
  float container_width;
  float LFB_cavity_height;

  float pre_remaining_platform;
  float post_remaining_platform;

  float item_height;
  float item_length;
  float item_width;
  bool item_shiny;

  float platform_difference;
  int expected_new_items_in_bag;
  float min_item_dimension;
  float max_item_dimension;

  std::shared_ptr<depthcam::aruco::MarkerDetectorContainer> pre_container;
  std::shared_ptr<fulfil::depthcam::aruco::MarkerDetectorContainer> post_container;

  cv::Mat pre_color_img;
  cv::Mat post_color_img;
  cv::Mat target_image;

  int grid_num_rows;
  int grid_num_cols;
  bool rotate_LFB_img;

  //depth comparison parameters
  float depth_baseline_average_threshold; //selected to avoid noise in pre/post comparison for empty LFBs
  float depth_factor_correction;
  float depth_total_threshold;

  //RGB comparison parameters
  int bg_sub_history; //length of history
  double bg_sub_variance_threshold; //Threshold on the squared distance between the pixel and the sample to decide whether a pixel is close to that sample
  bool bg_sub_detect_shadows; //If true, the algorithm will detect shadows and mark them. It decreases the speed a bit, so if you do not need this feature, set the parameter to false.
  int RGB_average_threshold;
  int RGB_total_threshold;

  float allowable_platform_difference; //in meters, between the pre and post images

  // Threshold based on HSV of the Fulfil Bag parameters
  double H_low;
  double H_high;
  double S_low;
  double S_high;
  double V_low ;
  double V_high;

  /*
   * Assigns values to prePostCompare class variables
   * Returns status code from check_inputs which is called within the function:
   * status code mapping defined in PrePostCompareErrorCodes
   */
  int populate_class_variables(std::shared_ptr<fulfil::depthcam::aruco::MarkerDetectorContainer> pre_container,
                               std::shared_ptr<fulfil::depthcam::aruco::MarkerDetectorContainer> post_container,
                               std::shared_ptr<fulfil::configuration::lfb::LfbVisionConfiguration> lfb_vision_config,
                               std::shared_ptr<nlohmann::json> pre_request_json,
                               std::shared_ptr<nlohmann::json> post_request_json,
                               std::shared_ptr<nlohmann::json> drop_target_json,
                               cv::Point2f target_center);

  int populate_class_variables_side_dispense(std::shared_ptr<fulfil::depthcam::aruco::MarkerDetectorContainer> pre_container,
      std::shared_ptr<fulfil::depthcam::aruco::MarkerDetectorContainer> post_container,
      std::shared_ptr<fulfil::configuration::lfb::LfbVisionConfiguration> lfb_vision_config,
      std::shared_ptr<nlohmann::json> pre_request_json,
      std::shared_ptr<nlohmann::json> post_request_json);

  int process_depth();
  int process_RGB();
  int process_absolute_difference(std::shared_ptr<fulfil::configuration::lfb::LfbVisionConfiguration> lfb_vision_config);

  //returns the percentage of the detected dispensed item that overlaps with the dispense target
  int process_target(cv::Point2f target_center, cv::Mat item_result_map);

  /*
   * Takes input parameters from JSON and checks if they are valid. Returns a status code:
   * 0 - no issues
   * 3 - Pre/Post Comparison: Invalid Item Dimensions
   * 4 - Pre/Post Comparison: Invalid Platform Value
   * 5 - Pre/Post Comparison: Platform Inconsistency
   */
  int check_inputs(float pre_remaining_platform, float post_remaining_platform,
                    float item_height, float item_length, float item_width);

 public:
  //constructor
  PrePostCompare(int visualize, std::shared_ptr<DropZoneSearcher> drop_zone_searcher);

  cv::Mat get_affine_registration_transform(std::shared_ptr<fulfil::depthcam::aruco::MarkerDetectorContainer> container, std::shared_ptr<fulfil::depthcam::aruco::MarkerDetectorContainer> post_container);

  std::shared_ptr<cv::Mat> result_mat;
  /**
   *
   * @param pre_session
   * @param post_session
   * @param lfb_vision_config
   * @param pre_request_json
   * @param post_request_json
   * @param target_center:  X,Y coordinates must be in meters (local bag coordinate system)
   *
   * Assigns an image to a cv::Mat pointer: image if new item detected, nullptr if not
   * Returns a result code:
   * -1: unknown/unassigned error
   * 0: no item detected
   * 1: item detected
   * 2: Caught error, indicates not enough markers detected in pre or post image
   * 3: Pre/Post Comparison: Invalid Item Dimensions
   * 4: Pre/Post Comparison: Invalid Platform Value
   * 5: Pre/Post Comparison: Platform Inconsistency
   */
  int run_comparison(std::shared_ptr<fulfil::depthcam::aruco::MarkerDetectorContainer> pre_container,
                     std::shared_ptr<fulfil::depthcam::aruco::MarkerDetectorContainer> post_container,
                     std::shared_ptr<fulfil::configuration::lfb::LfbVisionConfiguration> lfb_vision_config,
                     std::shared_ptr<nlohmann::json> pre_request_json,
                     std::shared_ptr<nlohmann::json> post_request_json,
                     std::shared_ptr<nlohmann::json> drop_target_json,
                     cv::Point2f target_center,
                     std::shared_ptr<cv::Mat> *result_mat,
                     int *item_target_overlap_ptr);

  /**
  * Evaluates the area of the risk map against the bag coverage threshold
  * Returns true if the damage prone item coverage is more than the threshold
  */
  bool check_damage_area(cv::Mat risk_map, float bag_coverage_threshold);

  int run_comparison_side_dispense(std::shared_ptr<fulfil::depthcam::aruco::MarkerDetectorContainer> pre_container,
      std::shared_ptr<fulfil::depthcam::aruco::MarkerDetectorContainer> post_container,
      std::shared_ptr<fulfil::configuration::lfb::LfbVisionConfiguration> lfb_vision_config,
      std::shared_ptr<nlohmann::json> pre_request_json,
      std::shared_ptr<nlohmann::json> post_request_json,
      std::shared_ptr<cv::Mat>* result_mat);

  double distance(const cv::Point2f& p1, const cv::Point2f& p2);

  cv::Mat calculate_roi(cv::Mat image, std::shared_ptr<fulfil::configuration::lfb::LfbVisionConfiguration> lfb_vision_config);
};

  namespace pre_post_compare_error_codes {
    enum PrePostCompareErrorCodes {
      Success = 0,
      NoMarkersDetected = 1,
      NotEnoughMarkersDetected = 2,
      InvalidItemDimensions = 3,
      InvalidRequest = 4,
      InconsistentPlatform = 5,
      UnspecifiedError = 10,
      HomographyTransformNotCompleted = 13,
      AffineTransformationUnstarted = 14,
      InvalidComparisonParameters = 15
    };
  } // namespace pre_post_compare_error_codes
} // namespace fulfil::dispense::drop

#endif // FULFIL_DISPENSE_SRC_PRE_POST_COMPARE_H_