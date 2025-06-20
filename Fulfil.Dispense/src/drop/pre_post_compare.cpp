
#include <tuple>
#include <bits/stdc++.h>
#include <memory>
#include <vector>
#include <Fulfil.CPPUtils/commands/dc_api_error_codes.h>
#include <Fulfil.CPPUtils/eigen/matrix3d_predicate.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.DepthCam/point_cloud.h>
#include <Fulfil.DepthCam/visualization.h>
#include <Fulfil.Dispense/drop/drop_grid.h>
#include <Fulfil.Dispense/drop/drop_result.h>
#include <Fulfil.Dispense/drop/drop_zone_searcher.h>
#include <Fulfil.Dispense/drop/pre_post_compare.h>
#include <Fulfil.Dispense/visualization/live_viewer.h>


using fulfil::configuration::lfb::LfbVisionConfiguration;
using fulfil::depthcam::aruco::Container;
using fulfil::depthcam::aruco::MarkerDetector;
using fulfil::depthcam::aruco::MarkerDetectorContainer;
using fulfil::depthcam::pointcloud::LocalPointCloud;
using fulfil::depthcam::pointcloud::PixelPointCloud;
using fulfil::depthcam::pointcloud::PointCloud;
using fulfil::depthcam::Session;
using fulfil::depthcam::visualization::SessionVisualizer;
using fulfil::dispense::drop::DropGrid;
using fulfil::dispense::drop::DropResult;
using fulfil::dispense::drop::PrePostCompare;
using fulfil::dispense::drop::pre_post_compare_error_codes::PrePostCompareErrorCodes;
using fulfil::dispense::visualization::LiveViewer;
using fulfil::utils::commands::dc_api_error_codes::DcApiErrorCode;
using fulfil::utils::commands::dc_api_error_codes::DcApiError;
using fulfil::utils::Logger;
using fulfil::utils::Point3D;

int evaluate_image(cv::Mat image, float threshold)
{
  int total_count = 0;
  for(int row = 0; row < image.rows; row++)
  {
    for(int col = 0; col < image.cols; col++)
    {
      if(image.at<float>(row,col) > threshold) total_count++;
    }
  }

  if(total_count == 0)
  {
    Logger::Instance()->Warn("No new items detected in bag!!!");
    return 0;
  }
  else
  {
    Logger::Instance()->Info("Item detected in bag! There were {} active squares in grid out of {}", total_count, image.rows*image.cols);
    return 1;
  }
}

PrePostCompare::PrePostCompare(int visualize, std::shared_ptr<DropZoneSearcher> drop_zone_searcher)
{
  /**
  *   Setup Visualizations + Windows for Displaying and Debug
  */
  std::shared_ptr<std::pair <int, int>> location_top_left = std::make_shared<std::pair <int, int>>(0,0);
  std::shared_ptr<std::pair <int, int>> location_top_right = std::make_shared<std::pair <int, int>>(1000, 0);
  std::shared_ptr<std::pair <int, int>> location_bottom_left = std::make_shared<std::pair <int, int>>(0,720);
  std::shared_ptr<std::pair <int, int>> location_bottom_right = std::make_shared<std::pair <int, int>>(1000,720);

  std::shared_ptr<std::pair <int, int>> window_size = std::make_shared<std::pair <int, int>>(960,540); // 960, 540   //1280,  720 for one monitor,  960, 540  for laptop
  std::shared_ptr<std::pair <int, int>> window_size2 = std::make_shared<std::pair <int, int>>(250,325);

  // Forcing wait on visualization is code 0, no wait is 1
  int wait_on_session_visualization = 1;
  this->visualize = visualize;

  this->drop_zone_searcher = drop_zone_searcher;

  std::shared_ptr<std::string> window_name_10 = std::make_shared<std::string>("Pre Image, Adjusted");
  std::shared_ptr<std::string> window_name_11 = std::make_shared<std::string>("Post Image");
  std::shared_ptr<std::string> window_name_12 = std::make_shared<std::string>("Pre Image, Depth");
  std::shared_ptr<std::string> window_name_13 = std::make_shared<std::string>("Post Image, Depth");
  std::shared_ptr<std::string> window_name_14 = std::make_shared<std::string>("Corners of Bag Region");

  std::shared_ptr<std::string> window_name_16 = std::make_shared<std::string>("Pre Image, Adjusted + Cropped");
  std::shared_ptr<std::string> window_name_17 = std::make_shared<std::string>("Post Image, Cropped");
  std::shared_ptr<std::string> window_name_18 = std::make_shared<std::string>("MOG2 Difference");
  std::shared_ptr<std::string> window_name_19 = std::make_shared<std::string>("Blob Detection");
  std::shared_ptr<std::string> window_name_20 = std::make_shared<std::string>("RGB Result");
  std::shared_ptr<std::string> window_name_21 = std::make_shared<std::string>("Placeholder 2");

  std::shared_ptr<std::string> window_name_22 = std::make_shared<std::string>("Pre Image, Avg Depth");
  std::shared_ptr<std::string> window_name_23 = std::make_shared<std::string>("Post Image, Avg Depth");
  std::shared_ptr<std::string> window_name_24 = std::make_shared<std::string>("Difference, Avg Depth");
  std::shared_ptr<std::string> window_name_25 = std::make_shared<std::string>("Difference, Processed");
  std::shared_ptr<std::string> window_name_26 = std::make_shared<std::string>("Depth Result");
  std::shared_ptr<std::string> window_name_27 = std::make_shared<std::string>("Target Grid");
  std::shared_ptr<std::string> window_name_28 = std::make_shared<std::string>("Item Hitting Target");

  std::shared_ptr<std::string> window_name_33 = std::make_shared<std::string>("Drop Target");
  std::shared_ptr<std::string> window_name_34 = std::make_shared<std::string>("Expected Item Locations");
  std::shared_ptr<std::string> window_name_35 = std::make_shared<std::string>("Placeholder 1");

  std::shared_ptr<Session> session = nullptr; //TODO: handle this better, only used here to initialize session visualizers. Refactor visualizers to not take sessions as input

  this->session_visualizer10 = std::make_shared<SessionVisualizer>(session, window_name_10, std::make_shared<std::pair <int, int>>(0,0), window_size2, wait_on_session_visualization);
  this->session_visualizer11 = std::make_shared<SessionVisualizer>(session, window_name_11, std::make_shared<std::pair <int, int>>(380,0), window_size2,
    wait_on_session_visualization);
  this->session_visualizer12 = std::make_shared<SessionVisualizer>(session, window_name_12, std::make_shared<std::pair <int, int>>(700,0), window_size2,
    wait_on_session_visualization);
  this->session_visualizer13 = std::make_shared<SessionVisualizer>(session, window_name_13, std::make_shared<std::pair <int, int>>(1020,0), window_size2,
    wait_on_session_visualization);
  this->session_visualizer14 = std::make_shared<SessionVisualizer>(session, window_name_14, std::make_shared<std::pair <int, int>>(1340,0), window_size2,
    wait_on_session_visualization);
  //this->session_visualizer15 = std::make_shared<SessionVisualizer>(session, window_name_15, std::make_shared<std::pair <int, int>>(1660,0), window_size2, wait_on_session_visualization);

  this->session_visualizer16 = std::make_shared<SessionVisualizer>(session, window_name_16, std::make_shared<std::pair <int, int>>(0,375), window_size2,
    wait_on_session_visualization);
  this->session_visualizer17 = std::make_shared<SessionVisualizer>(session, window_name_17, std::make_shared<std::pair <int, int>>(380,375), window_size2,
    wait_on_session_visualization);
  this->session_visualizer18 = std::make_shared<SessionVisualizer>(session, window_name_18, std::make_shared<std::pair <int, int>>(700,375), window_size2,
    wait_on_session_visualization);
  this->session_visualizer19 = std::make_shared<SessionVisualizer>(session, window_name_19, std::make_shared<std::pair <int, int>>(1020,375), window_size2,
    wait_on_session_visualization);
  this->session_visualizer20 = std::make_shared<SessionVisualizer>(session, window_name_20, std::make_shared<std::pair <int, int>>(1340,375), window_size2,
    wait_on_session_visualization);
  this->session_visualizer21 = std::make_shared<SessionVisualizer>(session, window_name_21, std::make_shared<std::pair <int, int>>(1660,375), window_size2,
    wait_on_session_visualization);

  this->session_visualizer22 = std::make_shared<SessionVisualizer>(session, window_name_22, std::make_shared<std::pair <int, int>>(0,735), window_size2,
    wait_on_session_visualization);
  this->session_visualizer23 = std::make_shared<SessionVisualizer>(session, window_name_23, std::make_shared<std::pair <int, int>>(380,735), window_size2,
    wait_on_session_visualization);
  this->session_visualizer24 = std::make_shared<SessionVisualizer>(session, window_name_24, std::make_shared<std::pair <int, int>>(700,735), window_size2,
    wait_on_session_visualization);
  this->session_visualizer25 = std::make_shared<SessionVisualizer>(session, window_name_25, std::make_shared<std::pair <int, int>>(1020,735), window_size2,
    wait_on_session_visualization);
  this->session_visualizer26 = std::make_shared<SessionVisualizer>(session, window_name_26, std::make_shared<std::pair <int, int>>(1340,735), window_size2,
    wait_on_session_visualization);
  this->session_visualizer27 = std::make_shared<SessionVisualizer>(session, window_name_27, std::make_shared<std::pair <int, int>>(1660,735), window_size2,
    wait_on_session_visualization);
  this->session_visualizer28 = std::make_shared<SessionVisualizer>(session, window_name_28, std::make_shared<std::pair <int, int>>(1980,735), window_size2,
    wait_on_session_visualization);
}

int PrePostCompare::check_inputs(float pre_remaining_platform, float post_remaining_platform,
                  float item_height, float item_length, float item_width)
{
  // remaining platform delta validation
  if (pre_remaining_platform != post_remaining_platform)
  {
    Logger::Instance()->Warn("Pre/Post Comparison: platform level is not the same during pre: {} and post: {}!",
                             pre_remaining_platform, post_remaining_platform);
    if(abs(pre_remaining_platform - post_remaining_platform) > this->allowable_platform_difference)
    {
      Logger::Instance()->Error("Pre/Post Comparison: Platform Inconsistency; Cam: LFB");
      return PrePostCompareErrorCodes::InconsistentPlatform;
    }
  }
  // pre/post remaining platform validation
  if (pre_remaining_platform < -0.01
    or pre_remaining_platform > 0.350
    or post_remaining_platform < -0.01
    or post_remaining_platform > 0.350)
  {
    Logger::Instance()->Error("Pre/Post Comparison: Invalid Platform Value; Vars: pre_remaining_platform = {}, "
                              "post_remaining_platform = {}; Cam: LFB", pre_remaining_platform, post_remaining_platform);
    return PrePostCompareErrorCodes::InvalidRequest;
  }
  // item dimension validation
  if (item_height < 0 or item_height > 1
    or item_length < 0 or item_length > 1
    or item_width < 0 or item_width > 1)
  {
    Logger::Instance()->Error("Pre/Post Comparison: Invalid Item Dimensions; Cam: LFB");
    return PrePostCompareErrorCodes::InvalidItemDimensions;
  }
  // no issues, return success code
  return PrePostCompareErrorCodes::Success;
}

int PrePostCompare::populate_class_variables(std::shared_ptr<MarkerDetectorContainer> pre_container,
                                             std::shared_ptr<MarkerDetectorContainer> post_container,
                                             std::shared_ptr<LfbVisionConfiguration> lfb_vision_config,
                                              std::shared_ptr<nlohmann::json> pre_request_json,
                                              std::shared_ptr<nlohmann::json> post_request_json,
                                              std::shared_ptr<nlohmann::json> drop_target_request_json,
                                              cv::Point2f target_center)
{
  /**
   *  Get relevant LFB dimensions and container dimensions
   */
  Logger::Instance()->Debug("Pre_Post_Compare.cpp populate class variables method begin");

  //Note: container_length and container_width are taken from configs here. They may be different from container dims
  //used in getting the pre or post containers, if those were extended to detect items sticking out over bag, for example
  this->container_length = lfb_vision_config->container_length;
  this->container_width = lfb_vision_config->container_width;
  this->LFB_cavity_height = lfb_vision_config->LFB_cavity_height;

  this->grid_num_rows = lfb_vision_config->grid_rows;
  this->grid_num_cols = lfb_vision_config->grid_cols;
  this->rotate_LFB_img = lfb_vision_config->rotate_LFB_viz;

  this->depth_baseline_average_threshold = lfb_vision_config->depth_baseline_average_threshold;
  this->depth_factor_correction = lfb_vision_config->depth_factor_correction;
  this->depth_total_threshold = lfb_vision_config->depth_total_threshold;

  if(depth_baseline_average_threshold == -1 or depth_factor_correction == -1 or depth_total_threshold == -1)
  {
    Logger::Instance()->Error("Invalid Depth Comparison Parameters; Cam: LFB");
    throw(PrePostCompareErrorCodes::InvalidComparisonParameters);
  }

  this->bg_sub_history = lfb_vision_config->bg_sub_history;
  this->bg_sub_variance_threshold = lfb_vision_config->bg_sub_variance_threshold;
  this->bg_sub_detect_shadows = lfb_vision_config->bg_sub_detect_shadows;
  this->RGB_average_threshold = lfb_vision_config->RGB_average_threshold;
  this->RGB_total_threshold = lfb_vision_config->RGB_total_threshold;

  this->allowable_platform_difference =  lfb_vision_config->allowable_platform_difference;

  this->H_low = lfb_vision_config->H_low;
  this->H_high = lfb_vision_config->H_high;
  this->S_low = lfb_vision_config->S_low;
  this->S_high = lfb_vision_config->S_high;
  this->V_low = lfb_vision_config->V_low;
  this->V_high = lfb_vision_config->V_high;

  if(bg_sub_history == -1 or bg_sub_variance_threshold == -1 or RGB_average_threshold == -1 or RGB_total_threshold == -1)
  {
    Logger::Instance()->Error("Invalid RGB Comparison Parameters; Cam: LFB");
    throw(PrePostCompareErrorCodes::InvalidComparisonParameters);
  }

  /**
   *  Get relevant info from the pre and post json request files (and convert to meters)
   **/

  this->pre_remaining_platform = ((*pre_request_json)["Remaining_Platform"].get<float>()) / 1000;
  this->post_remaining_platform = ((*post_request_json)["Remaining_Platform"].get<float>()) / 1000;

  this->item_height = ((*drop_target_request_json)["Lanes"].begin().value()["Item"]["H"].get<float>()) / 1000;
  this->item_length = ((*drop_target_request_json)["Lanes"].begin().value()["Item"]["L"].get<float>()) / 1000;
  this->item_width = ((*drop_target_request_json)["Lanes"].begin().value()["Item"]["W"].get<float>()) / 1000;
  this->item_shiny = (*drop_target_request_json)["Lanes"].begin().value()["Item"]["Shiny"].get<bool>();

  //check inputs here and get result
  int res = check_inputs(pre_remaining_platform, post_remaining_platform, item_height, item_length, item_width);
  if (res == 5) {
    auto platform = std::min(pre_remaining_platform, post_remaining_platform);
    Logger::Instance()->Warn("Hack to minimize platform errors. Setting both values to minimum platform height {}", platform);
    this->pre_remaining_platform = platform;
    this->post_remaining_platform = platform;
    res = 0;
  }

  this->platform_difference = post_remaining_platform - pre_remaining_platform;
  this->expected_new_items_in_bag = 1; //Can compare number in pre and post requests in future if add multi-dispense back
  this->min_item_dimension = std::min(item_height, std::min(item_length, item_width));
  this->max_item_dimension = std::max(item_height, std::max(item_length, item_width));

  Logger::Instance()->Debug("Pre image remaining platform: {}", pre_remaining_platform);
  Logger::Instance()->Debug("Post image remaining platform: {}", post_remaining_platform);
  Logger::Instance()->Debug("Expected number of new items in bag: {}", expected_new_items_in_bag);

  this->pre_container = pre_container;
  this->post_container = post_container;

  Logger::Instance()->Debug("Getting pre container color mat");
  this->pre_color_img = pre_container->get_color_mat()->clone();

  Logger::Instance()->Debug("Getting post container color mat");
  this->post_color_img = post_container->get_color_mat()->clone();

  return res; //return check input result
}

/*Populating side dispense configs*/
/*TO-DO - Remove unnecessary configs*/
int PrePostCompare::populate_class_variables_side_dispense(std::shared_ptr<MarkerDetectorContainer> pre_container,
    std::shared_ptr<MarkerDetectorContainer> post_container,
    std::shared_ptr<LfbVisionConfiguration> lfb_vision_config,
    std::shared_ptr<nlohmann::json> pre_request_json,
    std::shared_ptr<nlohmann::json> post_request_json)
{
    /**
     *  Get relevant LFB dimensions and container dimensions
     */
    Logger::Instance()->Debug("Pre_Post_Compare.cpp populate class variables method begin");

    //Note: container_length and container_width are taken from configs here. They may be different from container dims
    //used in getting the pre or post containers, if those were extended to detect items sticking out over bag, for example
    this->container_length = lfb_vision_config->container_length;
    this->container_width = lfb_vision_config->container_width;
    this->LFB_cavity_height = lfb_vision_config->LFB_cavity_height;

    this->grid_num_rows = lfb_vision_config->grid_rows;
    this->grid_num_cols = lfb_vision_config->grid_cols;
    this->rotate_LFB_img = lfb_vision_config->rotate_LFB_viz;

    this->depth_baseline_average_threshold = lfb_vision_config->depth_baseline_average_threshold;
    this->depth_factor_correction = lfb_vision_config->depth_factor_correction;
    this->depth_total_threshold = lfb_vision_config->depth_total_threshold;

    if (depth_baseline_average_threshold == -1 or depth_factor_correction == -1 or depth_total_threshold == -1)
    {
        Logger::Instance()->Error("Invalid Depth Comparison Parameters; Cam: LFB");
        throw(PrePostCompareErrorCodes::InvalidComparisonParameters);
    }

    this->bg_sub_history = lfb_vision_config->bg_sub_history;
    this->bg_sub_variance_threshold = lfb_vision_config->bg_sub_variance_threshold;
    this->bg_sub_detect_shadows = lfb_vision_config->bg_sub_detect_shadows;
    this->RGB_average_threshold = lfb_vision_config->RGB_average_threshold;
    this->RGB_total_threshold = lfb_vision_config->RGB_total_threshold;

    this->allowable_platform_difference = lfb_vision_config->allowable_platform_difference;

    this->H_low = lfb_vision_config->H_low;
    this->H_high = lfb_vision_config->H_high;
    this->S_low = lfb_vision_config->S_low;
    this->S_high = lfb_vision_config->S_high;
    this->V_low = lfb_vision_config->V_low;
    this->V_high = lfb_vision_config->V_high;

    if (bg_sub_history == -1 or bg_sub_variance_threshold == -1 or RGB_average_threshold == -1 or RGB_total_threshold == -1)
    {
        Logger::Instance()->Error("Invalid RGB Comparison Parameters; Cam: LFB");
        throw(PrePostCompareErrorCodes::InvalidComparisonParameters);
    }

    int res = check_inputs(pre_remaining_platform, post_remaining_platform, item_height, item_length, item_width);
    if (res == 5) {
        auto platform = std::min(pre_remaining_platform, post_remaining_platform);
        Logger::Instance()->Warn("Hack to minimize platform errors. Setting both values to minimum platform height {}", platform);
        this->pre_remaining_platform = platform;
        this->post_remaining_platform = platform;
        res = 0;
    }

    this->platform_difference = post_remaining_platform - pre_remaining_platform;
    this->expected_new_items_in_bag = 1; //Can compare number in pre and post requests in future if add multi-dispense back
    this->min_item_dimension = std::min(item_height, std::min(item_length, item_width));
    this->max_item_dimension = std::max(item_height, std::max(item_length, item_width));

    Logger::Instance()->Debug("Pre image remaining platform: {}", pre_remaining_platform);
    Logger::Instance()->Debug("Post image remaining platform: {}", post_remaining_platform);
    Logger::Instance()->Debug("Expected number of new items in bag: {}", expected_new_items_in_bag);

    bool is_first_dispense = post_request_json->value("Is_First_Dispense", false);
    this->pre_container = nullptr;
    Logger::Instance()->Debug("Getting pre container color mat");
    this->pre_color_img = cv::imread("/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/pre_dispense_images/color_image_ambient.png", cv::IMREAD_COLOR);
    
    if (!is_first_dispense) {
        this->pre_container = pre_container;
        this->pre_color_img = pre_container->get_color_mat()->clone();
    }

    this->post_container = post_container;

    Logger::Instance()->Debug("Getting post container color mat");
    this->post_color_img = post_container->get_color_mat()->clone();

    if (this->pre_color_img.size().empty()) {
        Logger::Instance()->Error("Received empty pre image for pre-post compare!");
        throw(DcApiErrorCode::EmptyPreImage);
    }
    if (this->post_color_img.size().empty()) {
        Logger::Instance()->Error("Received empty post image for pre-post compare!");
        throw(DcApiErrorCode::EmptyPostImage);
    }

    return res; //return check input result
}

cv::Mat PrePostCompare::get_affine_registration_transform(std::shared_ptr<MarkerDetectorContainer> pre_container,
                                                          std::shared_ptr<MarkerDetectorContainer> post_container)
{
    Logger::Instance()->Debug("Getting PRE container markers for affine transform");
    std::vector<std::shared_ptr<fulfil::depthcam::aruco::Marker>> pre_valid_markers = *(pre_container->get_markers());
    Logger::Instance()->Debug("Getting POST container markers for affine transform");
    std::vector<std::shared_ptr<fulfil::depthcam::aruco::Marker>> post_valid_markers = *(post_container->get_markers());

    // map with keys as marker IDs and values as the coordinate of the center of the marker
    std::map<int, cv::Point2f> pre_coordinates;
    for(auto i = 0; i < pre_valid_markers.size(); i++)
    {
        std::shared_ptr<fulfil::depthcam::aruco::Marker> marker = pre_valid_markers.at(i);

        int marker_id = marker->get_id();
        float x_coordinate = marker->get_coordinate(depthcam::aruco::Marker::center)->x;
        float y_coordinate = marker->get_coordinate(depthcam::aruco::Marker::center)->y;

        pre_coordinates[marker_id] = cv::Point2f(x_coordinate, y_coordinate);
    }

    std::vector<cv::Point2f> pre_image_points;
    std::vector<cv::Point2f> post_image_points;

    int points_added = 0;
    bool left_included = false;
    bool right_included = false;

    for(int i = 0; i < post_valid_markers.size(); i++)
    {
        if (points_added == 3) break;
        std::shared_ptr<fulfil::depthcam::aruco::Marker> marker = post_valid_markers.at(i);
        int marker_id = marker->get_id();

        if (points_added == 2) //if adding the final point, make sure both sides of LFB markers are covered
        {
            if (!left_included and (marker_id == 1 or marker_id == 2 or marker_id == 3 or marker_id == 4)) continue;
            if (!right_included and (marker_id == 0 or marker_id == 7 or marker_id == 6 or marker_id == 5)) continue;
        }

        if (pre_coordinates.count(marker_id) > 0) //if marker ID is listed in pre_image coordinates map, add coordiantes to both points vectors
        {
            Logger::Instance()->Debug("Marker ID {} is present in both images, adding to points vectors", marker_id);

            float x_coordinate = marker->get_coordinate(depthcam::aruco::Marker::center)->x;
            float y_coordinate = marker->get_coordinate(depthcam::aruco::Marker::center)->y;

            pre_image_points.push_back(pre_coordinates[marker_id]);
            post_image_points.push_back(cv::Point2f(x_coordinate, y_coordinate));

            left_included = left_included or (marker_id == 0 or marker_id == 7 or marker_id == 6 or marker_id == 5);
            right_included = right_included or (marker_id == 1 or marker_id == 2 or marker_id == 3 or marker_id == 4);
            points_added++;
        }
    }

    if (pre_image_points.size() != 3)
    {
        Logger::Instance()->Error("Homography Transform Not Completed; Cam LFB");
        throw(PrePostCompareErrorCodes::HomographyTransformNotCompleted);
    }

    if (!left_included or !right_included)
    {
        Logger::Instance()->Error("Unable To Do Affine Transformation; Cam LFB");
        throw(PrePostCompareErrorCodes::AffineTransformationUnstarted);
    }

    Logger::Instance()->Trace("Starting affine transform");
    cv::Mat transform = getAffineTransform(pre_image_points, post_image_points);
    Logger::Instance()->Trace("Finished w/ affine transform");

    return transform;
}
/**
* Comparison function to sort the vector elements
* by first element of tuples in descending order
**/
bool sort_desc(const std::tuple<float, int, int>& a,
               const std::tuple<float, int, int>& b)
{
  return (std::get<0>(a) > std::get<0>(b));
}

void find_k_largest( cv::Mat &img, int k, std::vector<std::tuple<float, int, int>> &result)
{
  //1. Augment into (z, row, col)
  int imgR = img.rows;
  int imgC = img.cols;
  std::vector<std::tuple<float, int, int>> aug(imgR * imgC);
  for (int r = 0; r < imgR; ++r)
  {
    for (int c = 0; c < imgC; ++c)
    {
      aug[r * imgC + c] = std::make_tuple(img.at<float>(r,c), r, c);
    }
  }
  //2. Sort!
  sort(aug.begin(), aug.end(), sort_desc);
  //3. Take the top k elements
  for (int i = 0; i < k; ++i)
  {
    result[i] = aug[i];
  }
}

int PrePostCompare::process_RGB()
{
  /**
   *  Get transformation between two LFB marker systems, and adjust pre RGB image accordingly
   **/
  cv::Mat RGB_transform = get_affine_registration_transform(pre_container, post_container);

  //cv::Mat RGB_homography = findHomography(pre_image_points, post_image_points, cv::RANSAC);
  // pre_image is translated + rotated so that LFB aligns with post_image LFB
  cv::Mat adjusted_pre_img = cv::Mat::zeros( pre_color_img.rows, pre_color_img.cols, pre_color_img.type() );
  warpAffine(pre_color_img, adjusted_pre_img, RGB_transform, pre_color_img.size());   // Warp source image to destination based on homography

  /**
   *  Use containers to detect pixels representing corners of LFB in post image (and adjust pre-image)
   */

  //calculate where center of LFB is on the post image (same as adjusted pre-image)
  std::shared_ptr<Eigen::Matrix3Xd> corner_data = std::make_shared<Eigen::Matrix3Xd>(3, 4);
  (*corner_data)(0,0) = 0 - container_width/2;
  (*corner_data)(1,0) = 0 - container_length/2;
  (*corner_data)(2,0) = 0;
  (*corner_data)(0,1) = 0 - container_width/2;
  (*corner_data)(1,1) = 0 + container_length/2;
  (*corner_data)(2,1) = 0;
  (*corner_data)(0,2) = 0 + container_width/2;
  (*corner_data)(1,2) = 0 + container_length/2;
  (*corner_data)(2,2) = 0;
  (*corner_data)(0,3) = 0 + container_width/2;
  (*corner_data)(1,3) = 0 - container_length/2;
  (*corner_data)(2,3) = 0;
  std::shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>> pixels = post_container->get_point_cloud(true)->
          as_local_cloud()->new_point_cloud(corner_data)->as_pixel_cloud()->get_data();

  cv::Mat LFB_corner_graphic;
  post_color_img.copyTo(LFB_corner_graphic);
  int radius = 1;
  int thickness = 10;

  for(auto i = 0; i < pixels->size(); i++)
  {
    cv::circle(LFB_corner_graphic, *pixels->at(i), radius, cv::Scalar(0,255,0), thickness);
  }

  /**
   *  Only consider region of interest based on LFB pixel corners
   */

  //find min and max x from corner pixels
  float min_x = (*pixels->at(0)).x;
  float max_x = (*pixels->at(0)).x;
  float min_y = (*pixels->at(0)).y;
  float max_y = (*pixels->at(0)).y;

  for(int i = 1; i < pixels->size(); i++)
  {
    float x_coordinate = (*pixels->at(i)).x;
    float y_coordinate = (*pixels->at(i)).y;

    if (x_coordinate < min_x) min_x = x_coordinate;
    if (x_coordinate > max_x) max_x = x_coordinate;
    if (y_coordinate < min_y) min_y = y_coordinate;
    if (y_coordinate > max_y) max_y = y_coordinate;
  }

  float cropped_width = int(max_x - min_x);
  float cropped_height = int(max_y - min_y);

  Logger::Instance()->Debug("Cropped image width, length is: {}, {}", cropped_width, cropped_height);
  //Todo: set check on cropped image size. Should be consistent across images. Otherwise indicative of rotated LFB. Error handling required

  // Setup a rectangle to define your region of interest
  cv::Rect myROI(min_x, min_y, int(max_x - min_x), int(max_y - min_y));

  // Crop the full image so that image contained by the rectangle myROI
  // Note that this doesn't copy the data
  cv::Mat cropped_pre_img = adjusted_pre_img(myROI);
  cv::Mat cropped_post_img = post_color_img(myROI);

  /**
   * Threshold out the bag using HSV of both pre and post imgs.
   */
  cv::Mat pre_img_HSV, post_img_HSV;
  cv::Mat pre_img_threshold, post_img_threshold;

  cv::cvtColor(cropped_pre_img, pre_img_HSV, cv::COLOR_BGR2HSV);
  cv::cvtColor(cropped_post_img, post_img_HSV, cv::COLOR_BGR2HSV);

  cv::Mat pre_mask, pre_inverted_mask, post_mask, post_inverted_mask;
  cv::inRange(pre_img_HSV, cv::Scalar(H_low, S_low, V_low), cv::Scalar(H_high, S_high, V_high), pre_mask);
  cv::bitwise_not(pre_mask, pre_inverted_mask);
  cv::bitwise_and(cropped_pre_img, cropped_pre_img, pre_img_threshold, pre_inverted_mask);

  cv::inRange(post_img_HSV, cv::Scalar(H_low, S_low, V_low), cv::Scalar(H_high, S_high, V_high), post_mask);
  cv::bitwise_not(post_mask, post_inverted_mask);
  cv::bitwise_and(cropped_post_img, cropped_post_img, post_img_threshold, post_inverted_mask);

  /**
   *  BackgroundSubtractor Module
   */
  cv::Mat thresh_adj_MOG2_img;

  cv::Ptr<cv::BackgroundSubtractor> bgSubtractor2 =  cv::createBackgroundSubtractorMOG2(bg_sub_history, bg_sub_variance_threshold, bg_sub_detect_shadows);
  bgSubtractor2->apply(pre_img_threshold, thresh_adj_MOG2_img, 1.0); //0 means that the background model is not updated at all, 1 means that the background model is completely reinitialized from the last frame
  bgSubtractor2->apply(post_img_threshold, thresh_adj_MOG2_img, 0.0);

  cv::Mat thresh_opened_img;
  cv::morphologyEx(thresh_adj_MOG2_img, thresh_opened_img, cv::MORPH_OPEN, cv::getStructuringElement(0, cv::Size(3,3), cv::Point(-1,-1)));

  cv::Mat thresh_RGB_grid_result(grid_num_rows, grid_num_cols, CV_32F, cv::Scalar(0));
  cv::Mat thresh_RGB_average_value(grid_num_rows, grid_num_cols, CV_32F, cv::Scalar(0));

  int k = 3;//@TODO: this will depend on item size. For now change it here for easy experimenting.
  std::vector<std::tuple<float, int, int>> thresh_k_largest(k);
  //cv::resize(opened_img, RGB_grid_result, cv::Size(grid_num_cols, grid_num_rows));

  //if image rotation is required, as defined in LFB_config, need to rotate result img before populating result grid
  //TODO: remove this condition once orientation of camera is fixed in one orientation on future system generations
  if(rotate_LFB_img) cv::rotate(thresh_opened_img, thresh_opened_img, cv::ROTATE_90_CLOCKWISE);

  int num_row_pixels = thresh_opened_img.rows;
  int num_col_pixels = thresh_opened_img.cols;
  /**
   *  Resize RGB MOG2 image to grid size and populate values with average value of pixels in region from previous image
   **/
  int row_pixels_per_region = floor(float(num_row_pixels) / grid_num_rows);
  int col_pixels_per_region = floor(float(num_col_pixels) / grid_num_cols);

  int result_code = 0;
  int thresh_square_count = 0;
  float thresh_total_sum = 0;

  for(int i = 0; i <  grid_num_rows; i++)
  {
    for(int j = 0; j < grid_num_cols; j++)
    {
      int l_r = i * row_pixels_per_region;
      int l_c = j * col_pixels_per_region;

      //cv::rect input format: x coordinate top-left corner, y coordinate top-left corner, width, height
      int thresh_average_region_value = int(thresh_average_region_value = cv::mean(cv::Mat(thresh_opened_img, cv::Rect(l_c, l_r, col_pixels_per_region, row_pixels_per_region))).val[0]);
      thresh_RGB_average_value.at<float>(i,j) = thresh_average_region_value;
      thresh_total_sum += thresh_average_region_value;

      if(thresh_average_region_value > RGB_average_threshold)
      {
        thresh_RGB_grid_result.at<float>(i, j) = 1;
        thresh_square_count += 1;
        result_code = 1; //set result to TRUE for item detected
      }
    }
  }

  find_k_largest(thresh_RGB_average_value, k, thresh_k_largest);

  if(result_code == 0 and thresh_total_sum > RGB_total_threshold)
  {
    Logger::Instance()->Debug("RGB comparison: Item Detected due to total threshold check in thresholded bag image");
    // no cells > comparison value so highlight largest k
    for(int i = 0; i < thresh_k_largest.size(); ++i)
    {
      float value = std::get<0>(thresh_k_largest[i]);
      int row = std::get<1>(thresh_k_largest[i]);
      int col = std::get<2>(thresh_k_largest[i]);
      if (value > 0) thresh_RGB_grid_result.at<float>(row, col) = 1;
    }
    result_code = 1;
  }

  Logger::Instance()->Debug("RGB compare: Max square difference value: {}, total difference sum: {}", std::get<0>(thresh_k_largest[0]), thresh_total_sum);
  Logger::Instance()->Debug("RGB compare: {} squares above change threshold of {}", thresh_square_count, RGB_average_threshold);

  if(visualize)
  {
    this->session_visualizer10->display_image(std::make_shared<cv::Mat>(adjusted_pre_img));
    this->session_visualizer14->display_image(std::make_shared<cv::Mat>(LFB_corner_graphic));

    this->session_visualizer16->display_image(std::make_shared<cv::Mat>(cropped_pre_img));
    this->session_visualizer17->display_image(std::make_shared<cv::Mat>(cropped_post_img));
    this->session_visualizer18->display_image(std::make_shared<cv::Mat>(thresh_adj_MOG2_img));
    this->session_visualizer19->display_image(std::make_shared<cv::Mat>(thresh_opened_img));
    this->session_visualizer20->display_image(std::make_shared<cv::Mat>(thresh_RGB_grid_result));
  }
  this->result_mat = std::make_shared<cv::Mat>(thresh_RGB_grid_result);
  return result_code;
}

int PrePostCompare::process_depth()
{
  /**
   *  Depth difference analysis
   */
  Logger::Instance()->Debug("Getting pre container point cloud now");
  std::shared_ptr<LocalPointCloud> pre_local_point_cloud = pre_container->get_point_cloud(false)->as_local_cloud();
  Logger::Instance()->Debug("Getting post container point cloud now");
  std::shared_ptr<LocalPointCloud> post_local_point_cloud = post_container->get_point_cloud(false)->as_local_cloud();

  if(this->visualize)
  {
    std::shared_ptr<cv::Mat> image3 = session_visualizer12->display_points_with_local_depth_coloring(pre_local_point_cloud,
                                                                                                     std::make_shared<cv::Mat>(this->pre_color_img));
    std::shared_ptr<cv::Mat> image4 = session_visualizer13->display_points_with_local_depth_coloring(post_local_point_cloud,
                                                                                                     std::make_shared<cv::Mat>(this->post_color_img));
    session_visualizer12->display_image(image3);
    session_visualizer13->display_image(image4);
  }

  float pre_platform_in_LFB_coords = 0.0F - (LFB_cavity_height - pre_remaining_platform);
  float post_platform_in_LFB_coords = 0.0F - (LFB_cavity_height - post_remaining_platform);
  Logger::Instance()->Debug("Pre platform level is {}, post platform level is {}", pre_platform_in_LFB_coords, post_platform_in_LFB_coords);

  //note: these are important even if grayed out. The function modifies the point cloud even if the returned Point3Ds are not used
  //Point3D max_Z_point_pre = drop_zone_searcher->adjust_depth_detections(std::make_shared<cv::Mat>(pre_color_img), pre_local_point_cloud, 10000, pre_platform_in_LFB_coords, false); //TODO: this function needs simplifying/refactoring
  //Point3D max_Z_point_post = drop_zone_searcher->adjust_depth_detections(std::make_shared<cv::Mat>(post_color_img), post_local_point_cloud, 10000, post_platform_in_LFB_coords, false);

  std::shared_ptr<Eigen::Matrix3Xd> pre_local_data = pre_local_point_cloud->get_data();
  std::shared_ptr<Eigen::Matrix3Xd> post_local_data = post_local_point_cloud->get_data();

  DropGrid pre_drop_grid = DropGrid(container_width, container_length, grid_num_rows, grid_num_cols);
  DropGrid post_drop_grid = DropGrid(container_width, container_length, grid_num_rows, grid_num_cols);

  pre_drop_grid.populate_depth(pre_local_data);
  post_drop_grid.populate_depth(post_local_data);

  // commenting out to reduce computation, can re-comment in if the max grid is used again in the future
  //  cv::Mat pre_depth_max_grid = pre_drop_grid.depth_max_grid;
  //  cv::Mat post_depth_max_grid = post_drop_grid.depth_max_grid;
  //  cv::Mat difference_max_grid = post_depth_max_grid - pre_depth_max_grid - platform_difference; //factor in difference in platform heights during calculation

  cv::Mat pre_depth_average_grid = pre_drop_grid.depth_average_grid;
  cv::Mat post_depth_average_grid = post_drop_grid.depth_average_grid;
  cv::Mat difference_average_grid = post_depth_average_grid - pre_depth_average_grid - platform_difference; //factor in difference in platform heights during calculation

  /**
   *  Apply item dimension info to determine whether depth differences are significant or noise
   */

  //average depth processing
  float comparison_value = std::max(depth_factor_correction * this->min_item_dimension, depth_baseline_average_threshold);

  double total_value_sum = cv::sum(difference_average_grid)[0]; // Assumes single channel depth image (currently what is held in depth grid
  cv::Mat filtered_difference_grid = (difference_average_grid > comparison_value)/255; //(or more explicitly use threshold )
  double square_count = cv::sum(filtered_difference_grid)[0];
  filtered_difference_grid.convertTo(filtered_difference_grid, CV_32F);

  int k = 3; //@TODO: this will depend on item size. For now change it here for easy experimenting.
  std::vector<std::tuple<float, int, int>> k_largest(k);
  find_k_largest(difference_average_grid, k, k_largest);

  int result_code = 0;
  Logger::Instance()->Info("Depth compare: Max square difference value: {}, total difference cv sum: {}"
    , std::get<0>(k_largest[0]), total_value_sum);
  Logger::Instance()->Info("Depth compare: {} squares above change threshold of {}.", square_count, comparison_value);

  if(square_count > 0)
  {
    result_code = 1;
    Logger::Instance()->Info("Depth compare: Item detected in bag! There were {} active squares in grid", square_count);
  }
  else if (total_value_sum > depth_total_threshold)
  {
    Logger::Instance()->Info("Depth compare: Item detected in bag! Total depth sum difference was significant at: {}", total_value_sum);
    // no cells > comparison value so highlight largest k
    for(int i = 0; i < k_largest.size(); ++i)
    {
      float value = std::get<0>(k_largest[i]);
      int row = std::get<1>(k_largest[i]);
      int col = std::get<2>(k_largest[i]);
      if (value > 0) filtered_difference_grid.at<float>(row, col) = 1;
    }
    result_code = 1;
  }
  else
  {
    Logger::Instance()->Debug("Depth compare: No item detected in bag!!!");
  }

  //cv::Mat depth_target_result = filtered_difference_grid.mul(this->target_image); //TODO: do we want to multiply here?
  this->result_mat = std::make_shared<cv::Mat>(filtered_difference_grid);



  if(visualize)
  {
    this->session_visualizer22->display_image_normalized(std::make_shared<cv::Mat>(pre_depth_average_grid),
                                                         pre_platform_in_LFB_coords,
                                                         0.1,
                                                         0,
                                                         1);
    this->session_visualizer23->display_image_normalized(std::make_shared<cv::Mat>(post_depth_average_grid),
                                                         post_platform_in_LFB_coords,
                                                         0.1,
                                                         0,
                                                         1);
    this->session_visualizer24->display_image_normalized(std::make_shared<cv::Mat>(difference_average_grid),
                                                         0,
                                                         this->max_item_dimension,
                                                         0,
                                                         1);
    this->session_visualizer25->display_image(std::make_shared<cv::Mat>(filtered_difference_grid));

    //this->session_visualizer26->display_image(std::make_shared<cv::Mat>(depth_target_result));
  }
  return result_code;
}

//helper function to calculate distance between 2 points
double PrePostCompare::distance(const cv::Point2f& p1, const cv::Point2f& p2) {
    return std::sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
}

//cropping the pre/post images to filter accurate ROI for calculating absolute difference
cv::Mat PrePostCompare::calculate_roi(cv::Mat image, std::shared_ptr<LfbVisionConfiguration> lfb_vision_config) {
        cv::Mat region_of_interest;
        std::vector<cv::Point2f> shrink_hull2f;
        cv::Point2f point;
        shrink_hull2f.push_back(cv::Point2f(lfb_vision_config->left_inner_bot_wall_x_pixel, lfb_vision_config->top_inner_bot_wall_y_pixel)); //left upper corner pixels
        shrink_hull2f.push_back(cv::Point2f(lfb_vision_config->right_inner_bot_wall_x_pixel, lfb_vision_config->top_inner_bot_wall_y_pixel)); //right upper corner pixels
        shrink_hull2f.push_back(cv::Point2f(lfb_vision_config->left_inner_bot_wall_x_pixel, lfb_vision_config->bottom_inner_bot_wall_y_pixel)); //left lower corner pixels
        shrink_hull2f.push_back(cv::Point2f(lfb_vision_config->right_inner_bot_wall_x_pixel, lfb_vision_config->bottom_inner_bot_wall_y_pixel)); //right lower corner pixels
        //calculate lengths of the sides of the new ROI image
        /* top -> represents the upper edge for width of the image
        * bottom -> represents the lower edge for width of the image
        * left -> represents the left edge for length of the image
        * right -> represents the right edge for length of the image
        */
        double top = distance(shrink_hull2f[0], shrink_hull2f[1]);
        double bottom = distance(shrink_hull2f[2], shrink_hull2f[3]);
        double new_width = std::max(top, bottom);
        double left = distance(shrink_hull2f[0], shrink_hull2f[2]);
        double right = distance(shrink_hull2f[1], shrink_hull2f[3]);
        double new_height = std::max(left, right);
        std::vector<cv::Point2f> roiPoints;
        roiPoints.push_back(cv::Point2f(0, 0));
        roiPoints.push_back(cv::Point2f(new_width, 0));
        roiPoints.push_back(cv::Point2f(0, new_height));
        roiPoints.push_back(cv::Point2f(new_width, new_height));
        cv::Mat transformation_matrix = cv::getPerspectiveTransform(shrink_hull2f, roiPoints);
        cv::warpPerspective(image, region_of_interest, transformation_matrix, cv::Size(new_width, new_height));
        int height = region_of_interest.rows;
        int width = region_of_interest.cols;
        Logger::Instance()->Info("Region of Interest has height {} and width {}", std::to_string(height), std::to_string(width));
        return region_of_interest;
}

int PrePostCompare::process_absolute_difference(std::shared_ptr<LfbVisionConfiguration> lfb_vision_config)
{
    cv::Mat abs_diff;
    int result = 0;
    int non_zeros_pre = cv::countNonZero(pre_color_img.reshape(1));
    int non_zeros_post = cv::countNonZero(post_color_img.reshape(1));
    if (non_zeros_pre == non_zeros_post) {
        Logger::Instance()->Info("Pre & Post images are same");
    }
    cv::Mat pre_color = calculate_roi(this->pre_color_img, lfb_vision_config);
    cv::Mat post_color = calculate_roi(this->post_color_img, lfb_vision_config);
    if (pre_color.size().empty()) {
        Logger::Instance()->Error("Pre color image is empty!");
    }
    if (post_color.size().empty()) {
        Logger::Instance()->Error("Post color image is empty!");
    }
    cv::absdiff(pre_color, post_color, abs_diff);
    int non_zeros = cv::countNonZero(abs_diff.reshape(1));
    Logger::Instance()->Info("Non_zeros pixels from absolute difference between pre/post: {}", non_zeros);
    this->result_mat = std::make_shared<cv::Mat>(abs_diff);
    bool hasDiff = non_zeros > 0;
    if (hasDiff) {
        Logger::Instance()->Info("Difference found!! Item detected in post dispense");
        result = 1;
        
    }
    else {
        Logger::Instance()->Info("No Difference found!! No item detected in post dispense");
    }
    return result;
}

int PrePostCompare::process_target(cv::Point2f target_center, cv::Mat item_result_map)
{
  Logger::Instance()->Debug("Target Center:  X = {},  Y = {}", target_center.x, target_center.y);
  float shadow_width = item_width;
  float shadow_length = item_height;

  DropGrid target_drop_grid = DropGrid(container_width, container_length, grid_num_rows, grid_num_cols);
  cv::Mat target_grid_img = target_drop_grid.get_grid_from_target(target_center.x, target_center.y, shadow_length, shadow_width);

  this->target_image = target_grid_img;

  int item_grid_count = cv::countNonZero(item_result_map);
  int target_grid_count = cv::countNonZero(target_grid_img);
  cv::Mat item_target_overlap = item_result_map.mul(target_grid_img);
  int item_target_overlap_count = cv::countNonZero(item_target_overlap);
  int overlap_percentage = int(100*((item_grid_count > 0) ? float(item_target_overlap_count) / item_grid_count : 0));


  Logger::Instance()->Debug("Dispensed item took up {} squares in grid. Target took up {} squares in grid", item_grid_count, target_grid_count);
  Logger::Instance()->Debug("Target and Result overlap for {} squares", item_target_overlap_count);

  if(visualize)
  {
    std::shared_ptr<Point3D> XYZ_result = std::make_shared<Point3D>(target_center.x, target_center.y, 0); //Todo: get proper target Z rather than using 0
    // transform target to post_image LFB frame and overlay on adjusted_pre_image
    std::shared_ptr<cv::Mat> target_image = drop_zone_searcher->visualize_target(XYZ_result, pre_container, shadow_length, shadow_width, std::make_shared<cv::Mat>(pre_color_img), 1, 3);

    session_visualizer14->display_image(target_image);
    session_visualizer27->display_image(std::make_shared<cv::Mat>(target_grid_img));
    session_visualizer28->display_image(std::make_shared<cv::Mat>(item_target_overlap));
  }
  return overlap_percentage;
}

bool PrePostCompare::check_damage_area(cv::Mat risk_map, float bag_coverage_threshold) {
    bool risk_map_has_space = true;
    cv::Mat white_mask;
    int rows = risk_map.rows;
    int cols = risk_map.cols;
    float total_pixel_count = rows * cols;
    int white_pixels = 0;
    cv::inRange(risk_map, cv::Scalar(250, 250, 250), cv::Scalar(255, 255, 255), white_mask);
    white_pixels = cv::countNonZero(white_mask);
    float non_white_pixel_count = total_pixel_count - (float)white_pixels;
    Logger::Instance()->Debug("Total pixel count: {}", total_pixel_count);
    Logger::Instance()->Debug("White pixel count: {}", white_pixels);
    Logger::Instance()->Debug("Non white pixel count: {}", non_white_pixel_count);

    float pixel_ratio = non_white_pixel_count/total_pixel_count;
    Logger::Instance()->Debug("Pixel Ratio of non white vs total pixels: {}", pixel_ratio);

    if (pixel_ratio >= bag_coverage_threshold) risk_map_has_space = false;
    Logger::Instance()->Debug("Map has space for glass/metal: {}", risk_map_has_space);
    return risk_map_has_space;
}

 int PrePostCompare::run_comparison(std::shared_ptr<MarkerDetectorContainer> pre_container,
                                   std::shared_ptr<MarkerDetectorContainer> post_container,
                                   std::shared_ptr<LfbVisionConfiguration> lfb_vision_config,
                                   std::shared_ptr<nlohmann::json> pre_request_json,
                                   std::shared_ptr<nlohmann::json> post_request_json,
                                   std::shared_ptr<nlohmann::json> drop_target_json,
                                   cv::Point2f target_center,
                                   std::shared_ptr<cv::Mat> *result_mat_ptr,
                                   int *target_item_overlap_ptr)
{
  Logger::Instance()->Debug("PrePostCompare run_comparison method called");
  int variable_validation_check = populate_class_variables(pre_container, post_container, lfb_vision_config, pre_request_json, post_request_json, drop_target_json, target_center);
  if (variable_validation_check != 0) return variable_validation_check;

  if(visualize)
  {
    this->session_visualizer10->display_image(std::make_shared<cv::Mat>(pre_color_img));
    this->session_visualizer11->display_image(std::make_shared<cv::Mat>(post_color_img));
  }

  // default result is unspecified error, will be overwritten with actual result code unless an unexpected error occurs
  int result_code = PrePostCompareErrorCodes::UnspecifiedError;
  this->result_mat = nullptr;

  try
  {
    // code will be 0 for no item detected, or 1 for item detected
    result_code = process_depth(); //will return nullptr 0 if no item detected
    if(result_code != 1)
    {
      Logger::Instance()->Debug("Depth comparison detection resulted in No Item Detected! Trying RGB processing next");
      // result code will be 0 for no item detected, or 1 for item detected
      result_code = process_RGB();
    }

    Logger::Instance()->Debug("Done with comparison processing. Final result code is: {}", result_code);

    *result_mat_ptr = this->result_mat; //for passing the result mat back out into drop_manager

    int target_overlap_result = process_target(target_center, *this->result_mat);
    *target_item_overlap_ptr = target_overlap_result;
  }
  catch(...)
  {
    Logger::Instance()->Debug("Caught an error during comparison processing. Indicative of not enough markers detected in an image.");
    return PrePostCompareErrorCodes::NotEnoughMarkersDetected;
  }

  return result_code;
}

//Pre-Post Comparison Run for Side Dispense
int PrePostCompare::run_comparison_side_dispense(std::shared_ptr<MarkerDetectorContainer> pre_container,
    std::shared_ptr<MarkerDetectorContainer> post_container,
    std::shared_ptr<LfbVisionConfiguration> lfb_vision_config,
    std::shared_ptr<nlohmann::json> pre_request_json,
    std::shared_ptr<nlohmann::json> post_request_json,
    std::shared_ptr<cv::Mat>* result_mat_ptr)
{
    Logger::Instance()->Info("PrePostCompare run_comparison method called");
    int variable_validation_check = populate_class_variables_side_dispense(pre_container, post_container, lfb_vision_config, pre_request_json, post_request_json);

    if (visualize)
    {
        this->session_visualizer10->display_image(std::make_shared<cv::Mat>(pre_color_img));
        this->session_visualizer11->display_image(std::make_shared<cv::Mat>(post_color_img));
    }
    // default result is unspecified error, will be overwritten with actual result code unless an unexpected error occurs
    int result_code = DcApiErrorCode::UnspecifiedError;
    this->result_mat = nullptr;

    try
    {
        if (result_code != 1)
        {
            Logger::Instance()->Info("Starting Absolute Difference processing");
            // result code will be 0 for no item detected, or 1 for item detected
            result_code = process_absolute_difference(lfb_vision_config);
        }
        Logger::Instance()->Info("Done with comparison processing. Final result code is: {}", result_code);

        *result_mat_ptr = this->result_mat; //for passing the result mat back out into drop_manager
    }

    catch (DcApiError& e) {
        DcApiErrorCode error_id = e.get_status_code();
        Logger::Instance()->Info("Caught an error during comparison processing. {}", e.get_description());
        return error_id;
    }
    catch (...)
    {
        Logger::Instance()->Info("Caught an error during comparison processing. Indicative of not enough markers detected in an image.");
        return DcApiErrorCode::NotEnoughMarkersDetected;
    }

    return result_code;
}



