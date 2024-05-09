#include "Fulfil.Dispense/tray/tray_algorithm.h"
#include <Fulfil.DepthCam/point_cloud.h>
#include <Fulfil.DepthCam/aruco/kabsch_helper.h>
#include <Fulfil.Dispense/tray/tray_lane.h>
#include <Fulfil.CPPUtils/logging.h>

#include "Fulfil.Dispense/visualization/make_media.h"
#include <Fulfil.CPPUtils/timer.h>
#include <Fulfil.DepthCam/frame/filtering.h>
#include <Fulfil.DepthCam/frame/pixel_point_converter.h>
#include <Fulfil.Dispense/tray/measurement_helpers.h>
#include <Fulfil.Dispense/tray/tray.h>
#include <opencv2/cudaarithm.hpp>

#include <cmath>
#include <numeric>
#include <algorithm>
#include <utility>
#include <experimental/filesystem>



namespace std_filesystem = std::experimental::filesystem;
using fulfil::depthcam::PixelPointConverter;
using fulfil::utils::timing::Timer;
using fulfil::dispense::tray::Tray;
using fulfil::dispense::tray_processing::TrayAlgorithm;
using fulfil::dispense::tray_processing::FEDParams;
using fulfil::utils::Point3D;
using fulfil::depthcam::visualization::AdditiveSessionVisualizer;
using fulfil::depthcam::aruco::FixedTransformContainer;
using fulfil::depthcam::pointcloud::PointCloud;
using fulfil::dispense::tray::TrayLane;
using fulfil::utils::Logger;
using fulfil::utils::ini::IniSectionReader;
using fulfil::utils::eigen::Matrix3XdBuilder;
using fulfil::depthcam::pointcloud::LocalPointCloud;
using fulfil::depthcam::pointcloud::CameraPointCloud;
using fulfil::depthcam::pointcloud::PixelPointCloud;
namespace filterUtils = fulfil::depthcam::filtering;
typedef std::tuple<std::array<float,3>, std::array<float,3>, std::array<float,3>> FloatPoints;


TrayAlgorithm::TrayAlgorithm(const IniSectionReader &tray_config_reader)
    : tray_config_section{tray_config_reader},
      tray_width(tray_config_reader.at<float>("tray_width")), tray_length(tray_config_reader.at<float>("tray_length")),
      dispense_arm_height(tray_config_reader.get_value("dispense_arm_height", 0.039f)), num_calib_coordinates(tray_config_reader.get_value( "num_calib_coordinates", 4)),
      back_edge_of_tray_local_y(-1 * (this->tray_length / 2 )), relative_max_height(tray_config_reader.get_value("relative_max_height", 1.2f)),
      relative_min_height(tray_config_reader.get_value("relative_min_height", 0.8f)),
      absolute_min_search_height_cutoff(tray_config_reader.get_value("absolute_min_search_height_cutoff", 0.07f)),
      relative_search_height(tray_config_reader.get_value("relative_search_height",float(0.4f*this->relative_max_height + 0.6f*this->relative_min_height))),
      y_distance_search_limit(tray_config_reader.get_value("y_distance_search_limit", 0.620f)),
      wheel_diameter_correction_mm(tray_config_reader.get_value("wheel_diameter_correction_mm", 35.0f)),
      tongue_wheel_adjustment_mm(tray_config_reader.get_value("tongue_wheel_adjustment_mm", 8.0f)),

      save_tray_visualizations(tray_config_reader.get_value("flags", "save_tray_visualizations", true))
{}



double TrayAlgorithm::meters_from_tray_front(double y_meters) const {
    return (this->tray_length / 2)-y_meters;
}


/**
  *   Setup Visualizations + Windows for Displaying and debug_tray
  */
void TrayAlgorithm::initialize_visualizers(const std::shared_ptr<depthcam::Session> &session)
{
    int wait_until_key = 0; // Wait for user to hit key to before continuing execution
    std::shared_ptr<std::string> combo_viz_name = std::make_shared<std::string>("Debug Tray Algo");
    std::pair <int, int> window_size = std::pair <int, int>(960,540); // 960, 540   //1280,  720 for one monitor,  960, 540  for laptop
    this->combo_visualizer = std::make_shared<AdditiveSessionVisualizer>(session,
                                                                 combo_viz_name, std::pair <int, int>(20, 20), window_size, wait_until_key);
}

std::experimental::filesystem::path TrayAlgorithm::make_save_data_path(const std::string &id_sequence_step,
    const std::string &request_result_dir) const {
    std_filesystem::path save_vis_path{ tray_config_section.get_value("flags", "save_data_base_path", "/home/fulfil/Videos/lane_dispense_data")};
    save_vis_path /= request_result_dir;
    save_vis_path = make_media::paths::add_basedir_date_suffix_and_join(save_vis_path, std::string(id_sequence_step + ".jpg"));
    Logger::Instance()->Debug("Saving visualization to location:\n  {} ", std::string(save_vis_path));
    return save_vis_path;
}


void TrayAlgorithm::save_mask(const std::string &id_sequence_step, cv::Mat mask, const std::string &mask_result_dir)
{
    auto timer = utils::timing::Timer("TrayAlgorithm::save_mask " + id_sequence_step);
    cv::resize(mask, mask, cv::Size(), 0.5, 0.5);
    cv::imwrite(make_save_data_path(id_sequence_step,mask_result_dir), mask);
}

std::shared_ptr<LocalPointCloud> TrayAlgorithm::generate_local_traycloud(
    const std::shared_ptr<fulfil::depthcam::Session> &session,
    const Eigen::Affine3d& camera_to_mm_transform, float max_height_relative_to_tray_plane)
{
    return generate_local_traycloud(session, camera_to_mm_transform,
      this->tray_width, max_height_relative_to_tray_plane, 0, 0);
}


std::shared_ptr<LocalPointCloud> TrayAlgorithm::generate_local_traycloud(
    const std::shared_ptr<fulfil::depthcam::Session> &session,
    const Eigen::Affine3d& camera_to_mm_transform,
    float container_width, float max_height_relative_to_tray_plane,
    float center_x, float center_y, float min_height_relative_to_tray_plane)
{
  Logger::Instance()->Info("The clip height in pc is {} to {}",
                           min_height_relative_to_tray_plane, max_height_relative_to_tray_plane);

  std::shared_ptr<FixedTransformContainer> tray_roi = std::make_shared<FixedTransformContainer>(session,
      std::make_unique<Eigen::Affine3d>(camera_to_mm_transform), true,
      container_width, this->tray_length, center_x, center_y,
      max_height_relative_to_tray_plane, min_height_relative_to_tray_plane);
  return tray_roi->get_point_cloud(false)->as_local_cloud();
}

std::array<float, 3> TrayAlgorithm::get_max_height_in_tray(
    const std::shared_ptr<depthcam::pointcloud::LocalPointCloud> &tray_point_cloud)
{
  auto max_height_op2 = [] (const Eigen::Matrix3Xd& local_pc) {
    constexpr int vis_window = 10;
    min_coeff_visitor<Eigen::MatrixXd, vis_window> min_vis{};
    local_pc.row(2).visit(min_vis);
    std::stringstream  mvs ; mvs << min_vis;
    auto diff_btwn = min_vis.res.back() - min_vis.res[0];
    Logger::Instance()->Info("Max Idx={}, Value={:0.5f}, Min Idx={}, Value={:0.5f}, diff={:0.5f}\n{}",
        min_vis.col[0], min_vis.res[0],  min_vis.col.back(), min_vis.res.back(), diff_btwn, mvs.str());
    if (diff_btwn > 0.01) {
      Logger::Instance()->Warn("Large difference between max and value at {}: {}\n{}", vis_window, diff_btwn, mvs.str());
    }
    return min_vis.col.back();
  };

  // Max height call logic
  Eigen::Matrix3Xd local_data = *(tray_point_cloud->get_data());
  std::array<float, 3>  max_pt =  get_matrix_maximum(local_data, max_height_op2);
  return max_pt;
}

cv::Point2i log_max_height_in_tray(const PixelPointConverter &local_pix2pt,
  std::array<float, 3> max_pt,
  float expected_max)
{
  Logger::Instance()->Info("The max height is located at point ({:0.3f},{:0.3f},{:0.3f})", max_pt[0], max_pt[1], max_pt[2]);
  float max_detection = fulfil::measure::to_rounded_millimeters(-max_pt[2]);
    auto max_pixel = local_pix2pt.get_pixel_from_point(max_pt.data());
    Logger::Instance()->Info("Pixel location of max height is (x={},y={})", max_pixel.x, max_pixel.y);
    return max_pixel;
}

void TrayAlgorithm::add_calibration_pixels(const std::string& config_key){
  // Visualize calibration pixels
  auto get_pixel_vector = [&config_key, this](auto dim) { return this->tray_config_section.at<std::vector<int>>(config_key, dim);};

  std::vector<cv::Point2i> calibration_pixels {};
  auto xpix = get_pixel_vector("x"); auto ypix = get_pixel_vector("y");
  std::transform(xpix.cbegin(), xpix.cend(), ypix.cbegin(), std::back_inserter(calibration_pixels), [&] (int _x, int _y) {
    return cv::Point2i {_x,_y};
  });
}

void TrayAlgorithm::run_visualizers(cv::Mat mask,
  std::shared_ptr<LocalPointCloud> tray_point_cloud,
  const std::string &id_sequence_step,
  std::vector<cv::Point> detections,
  std::array<cv::Point, 2> center_line)
{
    if (!this->save_tray_visualizations)  return;
    auto timer = utils::timing::Timer("TrayAlgorithm::run_visualizers " + id_sequence_step);
    this->combo_visualizer->add_points_with_local_depth_coloring(tray_point_cloud); // main diff here is that we apply the cloud before the split
    cv::Mat result = this->combo_visualizer->get_current_base_image_state();
    this->combo_visualizer->apply_mask(mask);
    this->combo_visualizer->add_line(center_line[0], center_line[1]); // TODO This does not happen in the FED version: could make param
    if (!detections.empty()) {

        std::for_each(detections.cbegin(), detections.cend()-1, [this, &result](cv::Point pix)
          {
            this->combo_visualizer->add_circle(pix, 0, 255, 0, 3, 10);
            cv::circle(result, pix, 3, cv::Scalar(0,255,0), 10);
          });
        auto pix = detections.back();
        this->combo_visualizer->add_circle(pix, 255, 128, 0, 2, 7);
        cv::circle(result, pix, 2, cv::Scalar(0,128, 255), 7);
    }


    cv::hconcat(result, this->combo_visualizer->get_current_base_image_state(), result);
    save_mask(id_sequence_step, result, "fed_results");
}

results_to_vlsg::LaneItemDistance
TrayAlgorithm::get_first_item_distance_from_coordinate_matrix(
        const std::vector<Eigen::Vector3d> &item_edge_coordinates,
        const TrayLane &lane) const {
    if (item_edge_coordinates.empty()) return {lane.lane_id(), 0, -1};
    double y_dist = meters_from_tray_front(item_edge_coordinates.front().y());
    double item_distance = fulfil::measure::to_rounded_millimeters(y_dist);
    auto tongue_wheel_correction = (lane.has_tongue()) ? this->wheel_diameter_correction_mm + this->tongue_wheel_adjustment_mm : this->wheel_diameter_correction_mm;

    int item_corrected_distance = static_cast<int>(std::max(item_distance - tongue_wheel_correction, 0.0));
    int item_length = static_cast<int>(fulfil::measure::to_rounded_millimeters(item_edge_coordinates.front().y() - item_edge_coordinates.back().y()));
    if (y_dist > this->y_distance_search_limit) {  item_corrected_distance = -1; item_length = 0; }
     Logger::Instance()->Info("Distance of item using tray center reference frame {:0.3f}, where the back of tray is {:0.3f}. The item {} the lane boundary. "
                               "The distance in mm from front of tray is {}. After correcting for the wheel diameter (set to {}) the belt distance "
                               "sent to the vlsfw is {}.", y_dist, this->y_distance_search_limit, 
			       (y_dist < this->y_distance_search_limit) ? "fell within" : "EXCEEDED",
                               item_distance, tongue_wheel_correction, item_corrected_distance);
    return {lane.lane_id(), 0, item_corrected_distance, item_length};

}

//TODO Fix shared ptr
void TrayAlgorithm::scan_for_back_item_edge(cv::LineIterator& lane_center_iterator, int &index,
                                            const PixelPointConverter &local_pix2pt,
                                            const TrayLane &current_lane)
{
  Eigen::Vector3d depth_pt = local_pix2pt.get_point_from_pixel(lane_center_iterator.pos());
  auto get_clipped_y_limits = [&depth_pt, &current_lane, back_edge_of_tray=this->back_edge_of_tray_local_y] (float proportion_item_len) {
    return std::max((double)back_edge_of_tray,
                              depth_pt.y() - (current_lane.get_item_length_in_meters()* proportion_item_len)); }; // in m
    double init_jump = get_clipped_y_limits((current_lane.is_rigid()) ? 0.99 : 0.92);
    double min_y = get_clipped_y_limits((current_lane.is_rigid()) ? 1.02 : 1.035);

    auto height_boundaries = get_height_cut_offs(current_lane.get_item_height_in_meters());
    float min_height = height_boundaries[0]; float max_height = height_boundaries[2];

    auto continue_scan = [&](const Eigen::Vector3d& cur_pt, const cv::Point2i& cur_pix){
        return index < lane_center_iterator.count &&
            (cur_pt.y() > init_jump || ((-cur_pt.z() > min_height) && (cur_pt.y() > min_y) && (-cur_pt.z() < max_height)));};
   // If the line iterator is not at the end,
    // Y decreases going back to the tray so when y <= to jump, we have gone an item length
    // Keep scanning until there is a break in item detection conditions (since the jump may get caught on the back of the item)
    while (continue_scan(depth_pt, lane_center_iterator.pos())){
        ++lane_center_iterator, index++;
        depth_pt = local_pix2pt.get_point_from_pixel(lane_center_iterator.pos());
    }
}

//TODO break out iteration to a seperate function that handles the looping
// pass a predicate, action on success, action on fail, possibly an every action?

std::array<float, 4> fulfil::dispense::tray_processing::TrayAlgorithm::get_height_cut_offs(float current_item_height) const {
    auto relative_to_total_height = [&] (auto relative_height) { // TODO look up this pointer lambda rules
        return this->dispense_arm_height + relative_height * current_item_height; };
    // TODO pass in z val so can do at disp height and in tray
    float safe_absolute_height = this->tray_config_section.get_value("safe_absolute_height", 0.16f);
    float safe_search_cap = this->tray_config_section.get_value("safe_search_cap_height", 0.16f);
    float max_height = std::max(relative_to_total_height(this->relative_max_height), safe_absolute_height);
    float min_height = std::min(std::max(relative_to_total_height(this->relative_min_height), this->absolute_min_search_height_cutoff), safe_absolute_height);
    float search_height = std::min(std::max(relative_to_total_height(this->relative_search_height), this->absolute_min_search_height_cutoff), safe_search_cap);
    return {min_height, search_height, safe_absolute_height, max_height};
}

std::array<float, 2> fulfil::dispense::tray_processing::TrayAlgorithm::get_width_extents(float current_item_width, float lane_center)
{
  float relative_width_extents =  this->tray_config_section.get_value( "relative_width_extents", 0.36f);
  float min_x = lane_center - relative_width_extents* current_item_width;
  float max_x = lane_center + relative_width_extents*  current_item_width;
  return {min_x, max_x};
}
// Stable partition to index
// transform to pixel vec and possibly point
// log id and add visuals
// predicate for hit
// lambda defining actions for found, search and scan states

FEDParams TrayAlgorithm::get_fed_params(const fulfil::dispense::tray::TrayLane &current_lane, float lane_center)
{
  float item_height = current_lane.get_item_height_in_meters();
  auto&& [min_height, search_height, safe_height, max_height ]= get_height_cut_offs(item_height);
  auto&& [min_x, max_x ] = get_width_extents(current_lane.get_item_width_in_meters(), lane_center);

  float dead_zone_start = this->tray_config_section.get_value( "start_deadzone", 0.073f);
  float dead_zone_len = this->tray_config_section.get_value("len_deadzone", 0.010f);
  auto start_dead_zone = static_cast<float>(meters_from_tray_front(dead_zone_start));
  auto end_dead_zone = static_cast<float>(start_dead_zone - dead_zone_len);

  Logger::Instance()->Debug("Item Height {}, Dispense arm height {}, Min Height {}, Search Height {}, "
      "Safe height {}, Back tray {}, Max x {}, Min x {}, Dead Zone {} - {}.",
                            current_lane.get_item_height_in_meters(), this->dispense_arm_height, min_height,
                            search_height, safe_height, this->back_edge_of_tray_local_y, max_x, min_x, start_dead_zone, end_dead_zone);
  return FEDParams{item_height, min_height, search_height, safe_height, max_height,
    min_x, max_x, start_dead_zone, end_dead_zone};
}


std::tuple<std::vector<Eigen::Vector3d>, std::vector<cv::Point>>
  TrayAlgorithm::analyze_lane(cv::LineIterator lane_center_iterator,
  const PixelPointConverter &local_pix2pt,
  const TrayLane &current_lane,
    FEDParams params) {
    double num_over_tray_on_line = 0;
    double sum_over_tray = 0;
    auto invalid_depth = [&] (const Eigen::Vector3d& cur_pt) {
        if (cur_pt.z() < 0) {
            num_over_tray_on_line++;
            sum_over_tray-= cur_pt.z();
        }
        return -cur_pt.z() < params.min_height || (-cur_pt.z() > params.max_height)
                 || (cur_pt.x() < params.min_x) || (cur_pt.x() > params.max_x);};

    uint dead_steps = 0; // REF PARAM
    auto log_dead_zone = [&](
                           //const cv::Point2i& pix,
                           bool in_dead_zone) {
      if (!in_dead_zone) return in_dead_zone;
      dead_steps++;
      Logger::Instance()->Debug( "Currently in dead zone! Have seen {} dead points", dead_steps);
      return in_dead_zone;
    };

    auto in_distance_dead_zone = [&]
        (const Eigen::Vector3d& cur_pt, const cv::Point2i& cur_pix, auto&& log_fn) {
        if (!current_lane.has_tongue()) return false;
        bool in_dead_zone = log_fn(//cur_pt, cur_pix,
          cur_pt.y() > params.end_dead_zone && cur_pt.y() < params.start_dead_zone && -cur_pt.z() < params.safe_height);
        return in_dead_zone;
    };

    std::vector<Eigen::Vector3d> edge_coordinates;
    std::vector<cv::Point> pixel_detections;
    Eigen::Vector3d back_edge_coordinate{};
    cv::Point back_pixel{};
    edge_coordinates.reserve(current_lane.get_num_items()+1);
    pixel_detections.reserve(current_lane.get_num_items()+1);
    auto log_front_edge_detection = [&]
        (const Eigen::Vector3d& pt, const cv::Point2i& pix, int step) {
      Logger::Instance()->Info("Lane {} a tongue. Item {} found on iteration {}. It is {:0.3f}mm away and {:0.2f}mm from belt. "
          "Item should be {:0.1f}mm tall. Detection located at pixel ({},{}) and point({:0.4f},{:0.4f},{:0.4f}), "
          "which passed filter {:0.3f} < x < {:0.3f} and {:0.3f} < -z < {:0.3f}"
            "\n    Current mean over tray is {:0.3f}", (current_lane.has_tongue()) ? "has" : "lacks",
          edge_coordinates.size(), step, meters_from_tray_front(pt.y())*1000,
          (-pt.z() - this->dispense_arm_height)*1000, params.item_height*1000, pix.y, pix.x,
          pt.x(), pt.y(), pt.z(), params.min_x, params.max_x, params.min_height, params.max_height, sum_over_tray/num_over_tray_on_line);
      edge_coordinates.push_back(pt);
      pixel_detections.push_back(pix);
    };
    auto log_back_edge_detection = [&] (cv::Point pix, int step) {
        if (edge_coordinates.size() != 1) { return ; }
        back_edge_coordinate = local_pix2pt.get_point_from_pixel(pix);
        back_pixel = pix;
        auto detected_length = (edge_coordinates.front().y()-back_edge_coordinate.y());
        Logger::Instance()->Info("BACK EDGE: Item 0 is {}. Found back on iteration {} between ({:0.4f}, {:0.4f}), and {:0.3f}mm away from the front.\n"
                                 "  |----> Expected length is {:0.4f}\n"
                                 "  |----> Detected length is {:0.4f}\n"
                                 "  |----> Measurement Error: {:0.4f}",
                             (current_lane.is_rigid()) ? "rigid" : "variable", step,
                             edge_coordinates.front().y(),back_edge_coordinate.y(),
                             meters_from_tray_front(back_edge_coordinate.y())*1000,
                                 current_lane.get_item_length_in_meters(), detected_length,
                                 detected_length - current_lane.get_item_length_in_meters());
    };

    Eigen::Vector3d depth_point{};
    // get rid of i and just check if equals to end / y pix limit?
    for(int i = 0; i < lane_center_iterator.count; i++, ++lane_center_iterator)
    {
        depth_point = local_pix2pt.get_point_from_pixel(lane_center_iterator.pos());
        if (!invalid_depth(depth_point) && !in_distance_dead_zone(depth_point, lane_center_iterator.pos(), log_dead_zone)) {
            log_front_edge_detection(depth_point, lane_center_iterator.pos(), i);
            scan_for_back_item_edge(lane_center_iterator, i, local_pix2pt, current_lane);
            log_back_edge_detection(lane_center_iterator.pos(), i);
        }
    }
    if (!edge_coordinates.empty()) {
        edge_coordinates.push_back(back_edge_coordinate);
        pixel_detections.push_back(back_pixel);
    }

    Logger::Instance()->Info("Mean of points on line over tray is {:0.3f}. Where {} out of {} were greater than zero.",
                             sum_over_tray/num_over_tray_on_line, num_over_tray_on_line, lane_center_iterator.count);

  return {edge_coordinates, pixel_detections };
}


Eigen::Affine3d
    TrayAlgorithm::create_camera_to_local_transform(const std::string &coordinate_config_prefix)
{
    Eigen::Matrix3Xd mm_fiducial_coordinates (3,num_calib_coordinates);
    load_eigen_matrix(mm_fiducial_coordinates, coordinate_config_prefix + "_tray_coordinates");

    Eigen::Matrix3Xd camera_fiducial_coordinates (3,num_calib_coordinates);
    load_eigen_matrix(camera_fiducial_coordinates, coordinate_config_prefix + "_camera_coordinates");
    fulfil::depthcam::KabschHelper helper;
    return helper.fit_transform_between_points(camera_fiducial_coordinates, mm_fiducial_coordinates);
}

// Should take in the lane_center_coordinates, the transform, and a point of delta in the 3 dimensions
[[nodiscard]] std::vector<std::vector<cv::Point2i>>
TrayAlgorithm::get_width_boundaries(Eigen::Matrix3Xd lane_center_coordinates,
                                     PixelPointConverter local_pix2pt,
                                     double max_item_width) const{
    Eigen::MatrixXd lane_width = Eigen::MatrixXd::Constant(1,lane_center_coordinates.cols(), max_item_width);
    lane_center_coordinates.row(0) = lane_center_coordinates.row(0) - lane_width/2;
    std::vector<cv::Point2i> lower_width_bounds {local_pix2pt.get_pixel_space_contour(lane_center_coordinates)};

    lane_center_coordinates.row(0) = lane_center_coordinates.row(0) + lane_width;
    std::vector<cv::Point2i> upper_width_bounds {local_pix2pt.get_pixel_space_contour(lane_center_coordinates)};
    
    // TODO I think this can be cleaned with new lane gutter output
    auto num_lanes = upper_width_bounds.size() / 2;
    std::vector<std::vector<cv::Point2i>> bounds{};
    bounds.reserve(lane_center_coordinates.cols());
    
    std::swap_ranges(upper_width_bounds.begin(), upper_width_bounds.begin() + num_lanes,
                   lower_width_bounds.begin() + num_lanes);
    std::transform(lower_width_bounds.cbegin(), lower_width_bounds.cend(), upper_width_bounds.cbegin(), std::back_inserter(bounds),
      [&](auto front, auto back ) { return std::vector{front, back}; });

    std::transform( bounds.cbegin(), bounds.cbegin() + num_lanes , bounds.cbegin() + num_lanes,
      bounds.begin(), [&](auto lower, auto upper ) {
        std::vector<cv::Point2i> v{};
          std::move(lower.begin(), lower.end(), std::back_inserter(v));
          std::reverse(upper.begin(), upper.end());
          std::move(upper.begin(), upper.end(), std::back_inserter(v));
        return v; });
    bounds.resize(num_lanes);
    bounds.shrink_to_fit();
    return bounds;

}

// REMOVE AFTER INITIAL TESTING
int verbose_tray_mask_log(double num_roi_pixels, double num_tongue_color_pixels, double bead_mask_rate_limit, bool is_tongue) {
    double percent_tongue_color = (num_tongue_color_pixels)/num_roi_pixels;
    bool guess_is_tongue = percent_tongue_color > bead_mask_rate_limit;
    Logger::Instance()->Debug("{}: The ROI is {:0.2f}% tongue_color which is {} than the cutoff {:0.2f}%. Found {} tongue_color "
        "pixels out of {} pixels in ROI. ", guess_is_tongue ? "TONGUE DETECTION" : "NO DETECTION",
        percent_tongue_color *100, guess_is_tongue ? "greater" : "less", bead_mask_rate_limit*100, num_tongue_color_pixels, num_roi_pixels);
    if (guess_is_tongue == is_tongue) { return 0; }
    Logger::Instance()->Warn("Tongue Status Mismatch: Expected a {} region, "
        "but predicted {}", is_tongue ? "TONGUE" : "BEAD",
        guess_is_tongue ?  "TONGUE" : "BEAD");

    return (!is_tongue && guess_is_tongue) ? 1 : -1;
}

// Remove debug has_tongue param
bool TrayAlgorithm::check_roi_for_tongue(const cv::Mat &tongue_color_mask,
    const std::vector<cv::Point2i> &roi_vertices, bool has_tongue,
    double masking_bead_cutoff)
{
    // todo should screen out high depth values, include num item / lane type
    cv::Mat roi_spatial_mask = depthcam::filtering::make_selection_by_convex_poly(
                                  tongue_color_mask.size(), roi_vertices);
    double number_pixels_in_roi = cv::countNonZero(roi_spatial_mask);
    depthcam::filtering::filter_out_selection(roi_spatial_mask, tongue_color_mask);
    double number_NON_tongue_color_roi_pixels = cv::countNonZero(roi_spatial_mask);
    double number_tongue_color_roi_pix = number_pixels_in_roi - number_NON_tongue_color_roi_pixels;
    auto get_roi_color = [&](int err_type) {
        if (err_type > 0) { return  cv::Scalar(0, 0, 255); }
        if (err_type < 0) { return cv::Scalar(0, 140, 255); }
        return cv::Scalar(0, 255, 0);
    };
    int verblog = verbose_tray_mask_log(number_pixels_in_roi, number_tongue_color_roi_pix, masking_bead_cutoff, has_tongue);
    this->combo_visualizer->add_perimeter(roi_vertices, get_roi_color(verblog) );

    // TODO should also think about no depth data type threshhold
    return (number_tongue_color_roi_pix/number_pixels_in_roi  > masking_bead_cutoff);
}

// TODO Beware -- > Tray can't be const'd because of the eigen mapping function, should check on that
std::vector<cv::Point2i>
    TrayAlgorithm::get_all_lane_center_pixels (Tray &current_tray,
        const PixelPointConverter &local_pix2pt,
        float center_line_height) const {
  Eigen::Matrix3Xd lane_center_coordinates =
      current_tray.get_tray_center_coordinates(this->tray_length, center_line_height);
  std::vector<cv::Point2i> lane_pixel_centers = std::vector<cv::Point2i>(local_pix2pt.get_pixel_space_contour(lane_center_coordinates));
  return lane_pixel_centers;

}

std::vector<cv::Point2i> TrayAlgorithm::get_all_lane_center_pixels(PixelPointConverter local_pix2pt,
    Eigen::Matrix3Xd lane_center_coordinates,
    const std::vector<tray::TrayLane> &tray_lanes, int lane_count) const{

  std::for_each(tray_lanes.cbegin(), tray_lanes.cend(), [&](TrayLane tl) {
    constexpr int row = 2;
    // TODO just get the search plane in a lambda and pass the height vector
    auto&& [min_height, search_height, safe_height, max_height ]= get_height_cut_offs(tl.get_item_height_in_meters());
    lane_center_coordinates(row, tl.lane_id()) =    -1.0*search_height;
    lane_center_coordinates(row, tl.lane_id()+lane_count) = -1.0*search_height;
  });
  std::vector<cv::Point2i> lane_pixel_centers {local_pix2pt.get_pixel_space_contour(lane_center_coordinates)};
  return lane_pixel_centers;
}

// TODO break off to own class for TCApi to DCApi comms
// TODO RENAME FN
// this is the fn used by tray validation

std::vector<TrayLane>
    make_lanes(const request_from_vlsg::TrayRequest &tray_vlsg_request){
  std::vector<TrayLane> tray_lanes{};
  std::transform(tray_vlsg_request.m_lanes.cbegin(), tray_vlsg_request.m_lanes.cend(),
      std::back_inserter(tray_lanes), [&] (dimensional_info::LaneInformation req_lane)
      {
        TrayLane lane {req_lane};
        return lane;
      }); // TODO FROM HERE
  return tray_lanes;
}



cv::LineIterator make_requested_center_iterator( const std::vector<cv::Point2i>& pixel_lane_centers,
    int lane_index, int num_lanes){
  auto line_front = pixel_lane_centers.at(lane_index);
  auto line_back = pixel_lane_centers.at(lane_index + num_lanes);
  constexpr float vertical_search_region_buffer = 1.05;
  constexpr float buffer_bias_to_back = 0.75;
  cv::LineIterator lane_center_iterator =
      filterUtils::make_lane_iterator(line_front, line_back,
          vertical_search_region_buffer, buffer_bias_to_back);
  Logger::Instance()->Debug("Line Iterator Created with {} points on line.", lane_center_iterator.count);
  return lane_center_iterator;
}

std::vector<cv::Point2i> TrayAlgorithm::get_center_pixels(const std::shared_ptr<fulfil::depthcam::Session> &session,
                                           Tray &current_tray, const request_from_vlsg::TrayRequest &tray_vlsg_request){
    std::string config_prefix = *(session)->get_serial_number() + tray_vlsg_request.m_context.get_calibration_mode_key();
    Eigen::Affine3d camera_to_mm_transform = create_camera_to_local_transform(config_prefix);
    PixelPointConverter local_pix2pt = PixelPointConverter(session, std::make_shared<Eigen::Affine3d>(camera_to_mm_transform), 10);

    // get lane spatial info
    return this->get_all_lane_center_pixels(current_tray, local_pix2pt, 0);
}

std::vector<cv::Point2i> TrayAlgorithm::get_center_pixels(
        PixelPointConverter local_pix2pt, const std::vector<TrayLane>& tray_lanes,
                                                          Tray &current_tray,
                                                          Eigen::Matrix3Xd& lane_center_coordinates){
    // get lane spatial info
    // TODO, this should just be an index return function. get_all_lane_center_pixels does almost the same thing
    auto center_pixels = this->get_all_lane_center_pixels(local_pix2pt, lane_center_coordinates, tray_lanes,current_tray.get_lane_count());
    Eigen::Matrix3Xd single_lane(3, 2);
    for (auto current_lane : tray_lanes) {// iterate over lanes instead
        single_lane << lane_center_coordinates.col(current_lane.lane_id()),
                lane_center_coordinates.col(current_lane.lane_id() + current_tray.get_lane_count());
        single_lane.row(2) =
                Eigen::RowVectorXd::Constant(single_lane.cols(), -1.0 * static_cast<double>(this->dispense_arm_height));
    }
    lane_center_coordinates = single_lane;
    return center_pixels;
}

std::tuple<std::vector<cv::Point2i>, std::vector<bool>> TrayAlgorithm::get_tongue_detections(
    const std::shared_ptr<fulfil::depthcam::Session> &session,
    Tray &current_tray,
    const request_from_vlsg::TrayRequest &tray_vlsg_request,
    std::vector<TrayLane> tray_lanes,
    cv::Mat tongue_color_mask)
{
  initialize_visualizers(session);
  bool tvr_request = (tray_vlsg_request.m_context.m_request_type == 5);

  std::string config_prefix = *(session)->get_serial_number() + tray_vlsg_request.m_context.get_calibration_mode_key();
  Eigen::Affine3d camera_to_mm_transform = create_camera_to_local_transform(config_prefix);
  PixelPointConverter local_pix2pt = PixelPointConverter(session, std::make_shared<Eigen::Affine3d>(camera_to_mm_transform), 10);

      // get lane spatial info
  Eigen::Matrix3Xd lane_center_coordinates = current_tray.get_tray_center_coordinates(this->tray_length, 0);
  std::vector<cv::Point2i> pixel_centers = [&]() {
    if (tvr_request) {
      return this->get_all_lane_center_pixels(current_tray, local_pix2pt, 0);
    }
    return this->get_all_lane_center_pixels(local_pix2pt, lane_center_coordinates, tray_lanes,current_tray.get_lane_count());
  }();
  if (!tvr_request) {
    std::vector<int> indices_of_interest; indices_of_interest.reserve(tray_lanes.size()*2);
    auto lane_indices = std::transform (tray_lanes.cbegin(), tray_lanes.cend(), std::back_inserter(indices_of_interest),
        [&] (const TrayLane lane) { return lane.lane_id();});
    Eigen::Matrix3Xd single_lane(3, 2);
    for (auto current_lane : tray_lanes) {// iterate over lanes instead
      single_lane << lane_center_coordinates.col(current_lane.lane_id()),
          lane_center_coordinates.col(current_lane.lane_id() + current_tray.get_lane_count());
      single_lane.row(2) =
          Eigen::RowVectorXd::Constant(single_lane.cols(), -1.0 * static_cast<double>(this->dispense_arm_height));
    }
    lane_center_coordinates = single_lane;
  }
  std::vector<bool> tongue_in_lane = validate_tongues_in_lane_on_tray(tongue_color_mask,
      tray_lanes,
      local_pix2pt,
      lane_center_coordinates,
      current_tray.get_max_item_width(), tray_vlsg_request.get_primary_key_id());
  return {pixel_centers, tongue_in_lane};

}

void get_min_max(cv::Mat m, int exp_valid_count){
  double minVal; double maxVal;
  cv::Point minLoc; cv::Point maxLoc;
  minMaxLoc( m, &minVal, &maxVal, &minLoc, &maxLoc );
  Logger::Instance()->Debug("Number of expected valid points: {}, and number valid points found in matrix: {}\n\tMin: ({},{})={}\n\tMax: ({},{})={}\n\tMean={}",
      exp_valid_count, cv::countNonZero(m),
      minLoc.x, minLoc.y, minVal, maxLoc.x, maxLoc.y, maxVal, cv::mean(m)[0]);
}

cv::Mat make_local_depth_frame(const std::shared_ptr<fulfil::depthcam::Session> &session,
    Eigen::Affine3f transform, cv::Size decimated_frame_size){
  // TODO remove hard code once decimation profile access enabled
  constexpr int decimation_factor = 8; // should this be a float and then floor / cast
  Eigen::Matrix3Xf depth_camera_cloud = session->get_point_cloud(true)->as_camera_cloud()->get_data()->cast<float>();
  cv::Mat local_aligned_depth_frame = cv::Mat::zeros(decimated_frame_size, CV_32FC1);

  auto depth_to_color_pt = [&](const Eigen::Vector3f& eigen_point) {
    std::array<float, 3> color_point = {0,0,0};
    rs2_transform_point_to_point(color_point.data(), session->get_depth_to_color_extrinsics().get(), eigen_point.data());
    return color_point;
  };

  auto project_to_color = [&](const std::array<float, 3>& color_point) {
    std::array<float, 2> color_pixel = {0,0};
    rs2_project_point_to_pixel(color_pixel.data(), session->get_color_stream_intrinsics().get(), color_point.data());
    return cv::Point2i {
      std::clamp(static_cast<int>(color_pixel[0])/decimation_factor, 0, decimated_frame_size.width),
      std::clamp(static_cast<int>(color_pixel[1])/decimation_factor,0,decimated_frame_size.height)};
  };
  auto local_depth = [&transform](const Eigen::Vector3f& eigen_point) { return (transform * eigen_point).z(); };
  auto is_valid = [](const Eigen::Vector3f& eigen_point) { return eigen_point.z() != 0; };

  int valid_count = 0;
  Logger::Instance()->Info("PC has {} points. Number invalid is {}, and {} valid.", depth_camera_cloud.cols(), (depth_camera_cloud.array().row(2) == 0).count(), (depth_camera_cloud.array().row(2) != 0).count());

  for (int i = 0 ; i < depth_camera_cloud.cols(); i++) {
    if (is_valid(depth_camera_cloud.col(i))) {
      auto color_pixel = project_to_color(depth_to_color_pt(depth_camera_cloud.col(i)));
      float depth = local_depth(depth_camera_cloud.col(i));
      local_aligned_depth_frame.at<float>(color_pixel) = depth;
      valid_count++;
    }
  }
  // need to make roi from 0 to (int)full_dim/8
  get_min_max(local_aligned_depth_frame, valid_count);
  return local_aligned_depth_frame;
}

cv::Mat get_aligned_local_depth_frame(const std::shared_ptr<fulfil::depthcam::Session> &session,
    Eigen::Affine3d transform) {
  float decimation_factor = 8; // Should get from decimation filter through sensor eventually
  cv::Size full_size = cv::Size(session->get_color_stream_intrinsics()->width, session->get_color_stream_intrinsics()->height);
  cv::Size padded_decimated_frame_size = session->get_point_cloud(true)->as_pixel_cloud()->get_size_point_cloud_as_frame(decimation_factor);
  Logger::Instance()->Debug("Padded decimated frame (magnitude {}) size (x,y) is: ({},{})", decimation_factor, padded_decimated_frame_size.width, padded_decimated_frame_size.height);
  cv::Size filled_decimated_frame_size = cv::Size(full_size.width/static_cast<int>(decimation_factor), full_size.height/static_cast<int>(decimation_factor));

  cv::Mat full_sized_local_frame;
  cv::Rect roi {cv::Point2i(0,0), filled_decimated_frame_size};
  cv::Mat local_decimated_frame = make_local_depth_frame(session, transform.cast<float>(), padded_decimated_frame_size)(roi);
  Logger::Instance()->Debug("Returned padded decimated frame size (x,y) is: ({},{}), and filled crop is: ({},{})",
      local_decimated_frame.size().width, local_decimated_frame.size().height,
      filled_decimated_frame_size.width, filled_decimated_frame_size.height);
  cv::resize(local_decimated_frame, full_sized_local_frame, full_size, 0, 0);

  return full_sized_local_frame;
}

void TrayAlgorithm::smooth_selection(cv::Mat &selection, bool dilate_first) {
    std::string find_unsafe_depth = dilate_first ?  "unsafe_depth_" : "safe_depth_";
    int dilation_kernel = this->tray_config_section.get_value("masking",find_unsafe_depth +"dilation_kernel_size", 5);
    int erosion_kernel = this->tray_config_section.get_value("masking",find_unsafe_depth +"erosion_kernel_size", 3);
    int dilation_iterations = this->tray_config_section.get_value("masking",find_unsafe_depth +"dilation_iterations", 1);
    int erosion_iterations = this->tray_config_section.get_value("masking",find_unsafe_depth +"erosion_iterations", 1);
    filterUtils::dilate_and_erode(selection, dilation_kernel, erosion_kernel, dilation_iterations, erosion_iterations, dilate_first);
}

// expects that negative is up
cv::Mat TrayAlgorithm::mark_distance_thresholds(const cv::Mat& depth_mat, float safe_depth_z, bool safe_distance){
  auto get_thresh_type = [] (bool up_direction_negative) {
    if (up_direction_negative) { return  cv::THRESH_BINARY_INV; }
    return cv::THRESH_BINARY;
  };
  cv::Mat clamped_mask;
  cv::threshold(depth_mat, clamped_mask, safe_depth_z, 255, get_thresh_type(safe_distance));
  clamped_mask.convertTo(clamped_mask, CV_8UC1);
  smooth_selection(clamped_mask, !safe_distance);
  return clamped_mask;
}

void visualize_height(cv::Mat& result_image,
  cv::Point absolute_max,  cv::Point selection,  cv::Point min_of_maxes) {
    cv::Scalar color{30, 25, 230};
                    cv::drawMarker(result_image, absolute_max, color, cv::MARKER_CROSS, 15, 5);  //red
    color[1] = 225; cv::drawMarker(result_image, min_of_maxes, color, cv::MARKER_CROSS, 15, 5);  // yellow
    color[2] = 10;  cv::drawMarker(result_image, selection, color, cv::MARKER_CROSS, 20, 10);  // green

  };

FloatPoints max_height_op_window(Eigen::Matrix3Xd local_pc){
    constexpr int vis_window = 10;
    fulfil::dispense::tray_processing::min_coeff_visitor<Eigen::MatrixXd, vis_window> min_vis{};
    local_pc.row(2).visit(min_vis);
    std::stringstream  mvs ; mvs << min_vis;
    auto diff_btwn = min_vis.res.back() - min_vis.res.front();
    std::vector<double> diffs{};
    std::adjacent_difference(min_vis.res.cbegin(), min_vis.res.cend(), std::back_inserter(diffs));
    std::stringstream diff_out;
    std::for_each(diffs.cbegin()+1, diffs.cend(), [&diff_out](const auto& n) {diff_out << std::fixed << std::setprecision(5) << n << ' ';}) ;
    auto max_diff = std::max_element(diffs.cbegin() + 1, diffs.cend());
    auto max_diff_idx = std::distance(diffs.cbegin(), max_diff);
    Logger::Instance()->Info("Max Idx={}, Value={:0.5f}, Min Idx={}, Value={:0.5f}, diff={:0.5f}, max diff=({}, {:0.5f}, {:0.5f})\nDif={}\n{}",
                             min_vis.col[0], min_vis.res[0],  min_vis.col.back(), min_vis.res.back(), diff_btwn, max_diff_idx,  *max_diff, min_vis.res[max_diff_idx], diff_out.str(), mvs.str());
    if (min_vis.col.back() == -1) {
        return FloatPoints({std::array<float,3>{0,0,0}, std::array<float,3>{0,0,0},std::array<float,3>{0,0,0}});
    }
    auto index_float = [&local_pc](int index) {
        auto p = local_pc.col(index).cast<float>();
        return std::array<float, 3>{p.x(), p.y(), p.z()};
    };

    return FloatPoints({index_float(min_vis.col.front()), index_float(min_vis.col[max_diff_idx]), index_float(min_vis.col.back())});
}

std::tuple<std::vector<tray_count_api_comms::LaneCenterLine>, std::vector<bool>, float>
    TrayAlgorithm::get_pixel_lane_centers_and_tongue_detections(
        const std::shared_ptr<fulfil::depthcam::Session> &session,
        const request_from_vlsg::TrayRequest &tray_vlsg_request,
        Tray &current_tray)
{
    Logger::Instance()->Debug("Getting Lane centers for {} request at position {}.", tray_vlsg_request.get_sequence_step(),
                                                                tray_vlsg_request.m_context.get_tray_position_name());

    // parse request to break out lane data
    std::vector<TrayLane> tray_lanes = make_lanes(tray_vlsg_request);
    auto max_height = tray_vlsg_request.expected_max_height();
     max_height =  max_height  == 0 ? -0.3: (-max_height/ 1000) * this->tray_config_section.get_value("height_detection_max_filter", 1.3f); // for max function

    // get spatial transform params
    std::string config_prefix = *(session)->get_serial_number() + tray_vlsg_request.m_context.get_calibration_mode_key();
    Eigen::Affine3d camera_to_mm_transform { create_camera_to_local_transform(config_prefix) };
    PixelPointConverter local_pix2pt = PixelPointConverter(session, std::make_shared<Eigen::Affine3d>(camera_to_mm_transform), 10);


    // select tongue area
    cv::Mat tongue_color_mask = make_tongue_color_selection(*session->get_color_mat());
    // get valid depth selection
    cv::Mat local_distance_frame = get_aligned_local_depth_frame(session, camera_to_mm_transform);
    cv::Mat safe_distance_selection = mark_distance_thresholds(local_distance_frame, this->tray_config_section.get_value("safe_tvr_tongue_depth", 0.01f), true);

    // Remove valid depths from tongue selection and evaluate tongues
    filterUtils::filter_out_selection( tongue_color_mask, safe_distance_selection);
    auto [lane_pixel_centers, tongue_detections] =
        get_tongue_detections(session, current_tray, tray_vlsg_request, tray_lanes, tongue_color_mask);



    // Create the lane center lines that will get sent to the count api
    std::vector<tray_count_api_comms::LaneCenterLine> center_line_objs {};
    std::transform(lane_pixel_centers.begin(), lane_pixel_centers.begin() + current_tray.get_lane_count(),
        lane_pixel_centers.begin() + current_tray.get_lane_count(), std::back_inserter(center_line_objs),
        [&](cv::Point2f front, cv::Point2f back ) {
          return tray_count_api_comms::LaneCenterLine{front.y, front.x, back.y, back.x};
        });
    /*****************/


    // get max dimension point
    auto tray_point_cloud =
        generate_local_traycloud(session, camera_to_mm_transform, max_height);

    auto [front_pt, space_pt, back_pt] = max_height_op_window(*(tray_point_cloud->get_data()));
    auto absolute_max = log_max_height_in_tray(local_pix2pt, front_pt, tray_vlsg_request.expected_max_height());
    auto selection = log_max_height_in_tray(local_pix2pt, space_pt, tray_vlsg_request.expected_max_height());
    auto min_of_maxes = log_max_height_in_tray(local_pix2pt, back_pt, tray_vlsg_request.expected_max_height());

    // Complete Visualization
    cv::Mat result = this->combo_visualizer->get_current_base_image_state();

    this->combo_visualizer->apply_mask(tongue_color_mask);
    this->combo_visualizer->add_points_with_local_depth_coloring(tray_point_cloud);
    cv::Mat masked_result = this->combo_visualizer->get_current_base_image_state();
    visualize_height(masked_result, absolute_max,  selection,  min_of_maxes);
    visualize_height(result, absolute_max,  selection,  min_of_maxes);


    cv::hconcat(result, masked_result, result);
    save_mask(tray_vlsg_request.m_context.get_id_tagged_sequence_step(), result, "tvr_results");
    return std::make_tuple(center_line_objs, tongue_detections, fulfil::measure::to_rounded_millimeters(std::max(-1*space_pt[2], 0.0f)));
}


std::tuple<results_to_vlsg::LaneItemDistance, std::vector<tray_count_api_comms::LaneCenterLine>, std::vector<bool>>
    TrayAlgorithm::run_tray_algorithm(
        const std::shared_ptr<fulfil::depthcam::Session>& session,
        const request_from_vlsg::TrayRequest &tray_vlsg_request,
        Tray current_tray)
{
  Logger::Instance()->Debug("Run Tray Algorithm Called with CommandID {}!", tray_vlsg_request.get_primary_key_id());

  // parse the request to break out tray lane data
  auto tray_lanes = make_lanes(tray_vlsg_request);
  auto current_lane = tray_lanes.front();
  FEDParams fed_params = get_fed_params(current_lane, current_tray.get_lane_center(current_lane.lane_id()));

  // get spatial transform params
  std::string config_prefix = *(session)->get_serial_number() + tray_vlsg_request.m_context.get_calibration_mode_key();
  Eigen::Affine3d camera_to_mm_transform = create_camera_to_local_transform(config_prefix);
  PixelPointConverter local_pix2pt = PixelPointConverter(session, std::make_shared<Eigen::Affine3d>(camera_to_mm_transform), 10);

  // get valid depth selection
  cv::Mat local_distance_frame = get_aligned_local_depth_frame(session, camera_to_mm_transform);
  cv::Mat safe_distance_selection = mark_distance_thresholds(local_distance_frame,-1 * fed_params.safe_height, true);
  //save_mask(tray_vlsg_request.m_context.get_id_tagged_sequence_step(), safe_distance_selection, "fed_safe_distance_selection");

  // Get location of aligned depth pixels below the minimum search height
  // select tongue area
  cv::Mat tongue_color_mask = make_tongue_color_selection(*session->get_color_mat());
  filterUtils::filter_out_selection( tongue_color_mask, safe_distance_selection);

    auto [pixel_lane_centers, tongue_detections] =
            get_tongue_detections(session, current_tray, tray_vlsg_request, tray_lanes, tongue_color_mask);


    // evaluate tongues
  //auto [pixel_lane_centers, tongue_detections] =
  //    get_tongue_detections(session, current_tray, tray_vlsg_request, tray_lanes, tongue_color_mask);
    Eigen::Matrix3Xd lane_center_coordinates = current_tray.get_tray_center_coordinates(this->tray_length, 0);
    std::cout << "LANE CENTERS 1:\n" << lane_center_coordinates << '\n';
    pixel_lane_centers =  get_center_pixels(local_pix2pt, tray_lanes, current_tray, lane_center_coordinates);
    std::cout << "LANE CENTERS 2:\n" << lane_center_coordinates << '\n';
    std::vector<std::vector<cv::Point2i>> lane_bounds = get_width_boundaries(lane_center_coordinates, local_pix2pt, current_tray.get_max_item_width());

    tongue_detections = validate_tongues_in_lane_on_tray(tongue_color_mask, lane_bounds, tray_lanes, tray_vlsg_request.get_primary_key_id());

  // Create the lane center lines that will get sent to the count api

  std::vector<tray_count_api_comms::LaneCenterLine> center_line_objs {};
  std::transform(pixel_lane_centers.begin(), pixel_lane_centers.begin() + current_tray.get_lane_count(),
      pixel_lane_centers.begin() + current_tray.get_lane_count(), std::back_inserter(center_line_objs),
      [&](cv::Point2f front, cv::Point2f back ) {
        return tray_count_api_comms::LaneCenterLine{front.y, front.x, back.y, back.x};
  });


  /*****************/

  /*** mutates depth frame ***/
  cv::Mat invalid_depth_selection = mark_distance_thresholds(local_distance_frame,
      -1 * this->dispense_arm_height, false);
  filterUtils::merge_selection( tongue_color_mask, invalid_depth_selection); // flip order here ?
  filterUtils::merge_selection(invalid_depth_selection, filterUtils::make_invalid_depth_selection(*(session->get_depth_mat(true)))); // may want to push below
    smooth_selection(tongue_color_mask, true); // or flip this up

  // TODO test over the local aligned frame directly instead
  filterUtils::filter_out_selection(*(session->get_depth_mat()), tongue_color_mask);
  cv::LineIterator lane_center_iterator = make_requested_center_iterator(
      pixel_lane_centers, current_lane.lane_id(), current_tray.get_lane_count());


  // Move precondition outside
  if (current_lane.get_num_items() == 0) {
      Logger::Instance()->Warn("Attempted to get FED on a lane that inventory has labelled as empty!");
      return std::make_tuple(results_to_vlsg::LaneItemDistance {current_lane.lane_id(), 5, -1}, center_line_objs, tongue_detections);
  }


  auto [edge_coordinates, detection_pixels]  =
      analyze_lane(lane_center_iterator, local_pix2pt, current_lane, fed_params);

  // Moved from analyze lane
  results_to_vlsg::LaneItemDistance lane_result =
    get_first_item_distance_from_coordinate_matrix(edge_coordinates, current_lane);
  Logger::Instance()->Info("Lane {}: & Closest item is {} mm away from front.",
    current_lane.lane_id(),
    lane_result.m_first_item_distance);
  // Moved from analyze lane

  auto tray_point_cloud = generate_local_traycloud(session,  camera_to_mm_transform,-0.4);
  run_visualizers(tongue_color_mask,
    tray_point_cloud,
tray_vlsg_request.m_context.get_id_tagged_sequence_step(),
    detection_pixels,
  {pixel_lane_centers.at(current_lane.lane_id()),
      pixel_lane_centers.at(current_lane.lane_id() + current_tray.get_lane_count())});

    lane_result.m_roi_points = tray_count_api_comms::LaneImageRegion(lane_bounds[0], 720.0F, 1280.0F);
    /*std::transform(lane_bounds.front().begin(), lane_bounds.front().end(), std::back_inserter(lane_result.m_roi_points),
                   [&](cv::Point2f pix) { return std::array<float, 2>{pix.y, pix.x}; });*/

  return std::make_tuple(lane_result, center_line_objs, tongue_detections);
}





cv::Mat TrayAlgorithm::make_tongue_color_selection(cv::Mat rgb_selection) {
    std::vector<int> low = this->tray_config_section.get_value("low_tongue_mask", std::vector<int>({150, 103, 110}));
    std::vector<int> high = this->tray_config_section.get_value( "high_tongue_mask", std::vector<int>({167, 255, 255}));
    cv::Mat tongue_color_mask = filterUtils::make_selection_by_color(
rgb_selection, cv::Scalar(low[0], low[1], low[2]), cv::Scalar(high[0], high[1], high[2]));
    return tongue_color_mask;
}

/**
low_tongue_mask=85 78 82
high_tongue_mask=131 255 255

 * */
// TODO break out as free function
// TODO move to caller
std::string polygon_print(const std::vector<cv::Point2i>& lane_outline) {
    std::stringstream printout; printout << "ROI vertices: ";
    for (auto v: lane_outline) {
        printout << "\n    (" << v.y << ", " << v.x <<")";
    }
    return printout.str();
}

std::vector<bool> TrayAlgorithm::validate_tongues_in_lane_on_tray(const cv::Mat &tongue_color_mask,
                                                                  const std::vector<std::vector<cv::Point2i>>& lane_bounds,
                                                                  const std::vector<TrayLane> &tray_lanes,
                                                                  const std::string &pkid)
{
    //std::vector<std::vector<cv::Point2i>> lane_bounds = get_width_boundaries(lane_center_coordinates, local_pix2pt, max_item_width);
    std::vector<bool> tongue_check_res; tongue_check_res.reserve(tray_lanes.size());
    float bead_mask_rate_limit = this->tray_config_section.get_value( "bead_tongue_color_limit", 0.08f);
    Logger::Instance()->Debug("Analyzing {} lane ROIs and {} tray lanes", lane_bounds.size(), tray_lanes.size());
    //todo add check for 13 lane
    std::transform(tray_lanes.cbegin(), tray_lanes.cend(), lane_bounds.cbegin(),
                    std::back_inserter(tongue_check_res), [&]
                   (const TrayLane cl, const std::vector<cv::Point2i>& lane_outline) {
          Logger::Instance()->Debug("Analyzing {} Lane {}, expecting {}. {}", pkid, cl.lane_id(), cl.has_tongue()? "tongue" : "bead",
                                    polygon_print(lane_outline));
           return check_roi_for_tongue(tongue_color_mask, lane_outline, cl.has_tongue(), bead_mask_rate_limit);
    });
    return tongue_check_res;
}


std::vector<bool> TrayAlgorithm::validate_tongues_in_lane_on_tray(const cv::Mat &tongue_color_mask,
                                                                  const std::vector<TrayLane> &tray_lanes,
                                                                  const PixelPointConverter &local_pix2pt,
                                                                  const Eigen::Matrix3Xd &lane_center_coordinates,
                                                                  float max_item_width,
                                                                  const std::string &pkid)
{
    std::vector<std::vector<cv::Point2i>> lane_bounds = get_width_boundaries(lane_center_coordinates, local_pix2pt, max_item_width);
    std::vector<bool> tongue_check_res; tongue_check_res.reserve(tray_lanes.size());
    float bead_mask_rate_limit = this->tray_config_section.get_value( "bead_tongue_color_limit", 0.08f);
    //todo add check for 13 lane
    std::transform(tray_lanes.cbegin(), tray_lanes.cend(), lane_bounds.cbegin(),
                   std::back_inserter(tongue_check_res), [&]
                           (const TrayLane cl, const std::vector<cv::Point2i>& lane_outline) {
                Logger::Instance()->Debug("Analyzing {} Lane {}, expecting {}", pkid, cl.lane_id(), cl.has_tongue()? "tongue" : "bead");
                return check_roi_for_tongue(tongue_color_mask, lane_outline, cl.has_tongue(), bead_mask_rate_limit);
            });
    return tongue_check_res;
}


void TrayAlgorithm::load_eigen_matrix(Eigen::Matrix3Xd &matrix, const std::string &section){
  std::vector<float> dims; // num_calib_coordinates x height x width x depth
  dims.reserve(this->num_calib_coordinates);
  // TODO remove repetition
  auto fill_matrix_dim = [&](std::string&& dim_name, int&& dim_code)
  {
    this->tray_config_section.fill_vector(section, dim_name, dims);
    for (int i = 0; i < this->num_calib_coordinates; i++)  matrix(dim_code, i) = dims[i];
    dims.clear();
  };
  fill_matrix_dim("x", 0);
  fill_matrix_dim("y", 1);
  fill_matrix_dim("depth", 2);

  // TODO remove logging
  std::stringstream matrix_string; matrix_string << matrix;
  Logger::Instance()->Trace("Loading the {} matrix:\n{}", section, matrix_string.str());
}



