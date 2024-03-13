//
// Created by steve on 5/12/20.
//

#ifndef FULFIL_DISPENSE_SRC_TRAY_HEIGHT_TRAY_ALGORITHM_H_
#define FULFIL_DISPENSE_SRC_TRAY_HEIGHT_TRAY_ALGORITHM_H_

#include <Fulfil.CPPUtils/inih/ini_utils.h>
#include <Fulfil.CPPUtils/eigen.h>
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include <Fulfil.DepthCam/frame/pixel_point_converter.h>
#include <Fulfil.DepthCam/visualization.h>
#include <Fulfil.Dispense/tray/tray.h>
#include <Fulfil.Dispense/tray/tray_lane.h>
#include <experimental/filesystem>
#include <memory>
#include <vector>


namespace fulfil::dispense::tray_processing {

/**
 * struct defining depth pixel and depth
 */

struct FEDParams {
  float item_height{};
  float min_height{};
  float search_height{};
  float safe_height{};
  float max_height{};
  float min_x{};
  float max_x{};
  double start_dead_zone{};
  double end_dead_zone{};
  FEDParams() = default;
};

template <typename Derived, size_t N>
struct top_n_coeff_visitor
{
  typedef typename Derived::Scalar Scalar;
  std::array<Eigen::Index, N> row{}, col{};
  std::array<Scalar, N> res{};
  Scalar init_val{};
  top_n_coeff_visitor() = default;
  EIGEN_DEVICE_FUNC
  template <typename CompOp>
  inline void attempt_insert(const Scalar& value, Eigen::Index i, Eigen::Index j, CompOp op) {
    auto p = std::upper_bound(this->res.begin(), this->res.end(), value, op);
    auto array_insert = [](auto &arr, int idx, auto val){
      auto elem = arr.begin()+idx;
      std::rotate(elem, arr.end() - 1, arr.end());
      *elem = val;
    };

    if (p != this->res.end()) {
      auto top_idx = std::distance(this->res.begin(), p);
      array_insert(this->res, top_idx, value);
      array_insert(this->row, top_idx, i);
      array_insert(this->col, top_idx, j);
    }
  }
  // Todo use comp op for first insert
  inline void init(const Scalar& value, Eigen::Index i, Eigen::Index j)
  {
    row.fill(-1);
    col.fill(-1);
    res.fill(init_val);
  }
};

template <typename Derived, size_t N>
std::ostream & operator << (std::ostream &out, const top_n_coeff_visitor<Derived, N> &top_res)
{
  auto log = [&](auto const& field, auto const& v, char end_char)
  {
    out << field;
    for (auto n : v)
      out << n << ' ';
    out << end_char;
  };
  log("Row: ", top_res.row, '\n');
  log("Col: ", top_res.col, '\n');
  log("Val: ", top_res.res, ' ');
  return out;
}

template <typename Derived, size_t N>
struct min_coeff_visitor : top_n_coeff_visitor<Derived, N>
{
  typedef typename Derived::Scalar Scalar;
  EIGEN_DEVICE_FUNC
  void operator() (const Scalar& value, Eigen::Index i, Eigen::Index j)
  {
    auto p = std::upper_bound(this->res.begin(), this->res.end(), value);
    auto array_insert = [](auto &arr, int idx, auto val){
      auto elem = arr.begin()+idx;
      std::rotate(elem, arr.end() - 1, arr.end());
      *elem = val;
    };

    if (p != this->res.end()) {
      auto top_idx = std::distance(this->res.begin(), p);
      array_insert(this->res, top_idx, value);
      array_insert(this->row, top_idx, i);
      array_insert(this->col, top_idx, j);
    }
  }
};


class TrayAlgorithm
{


 private:

   const fulfil::utils::ini::IniSectionReader&tray_config_section;
   float tray_width;  // in m, along x axis of tray (local frame)
   float tray_length; // in m, along y axis of tray (local frame)
   float dispense_arm_height;
   int num_calib_coordinates;
   float back_edge_of_tray_local_y; //offset of 0.02 is to ignore the raised bumpers at the ends of the trays
   float relative_max_height;
   float relative_min_height;
   float absolute_min_search_height_cutoff;
   float relative_search_height;
   float y_distance_search_limit;
   float wheel_diameter_correction_mm;
   float tongue_wheel_adjustment_mm;
   bool save_tray_visualizations;
   // TODO push to init
   std::shared_ptr<fulfil::depthcam::visualization::AdditiveSessionVisualizer> combo_visualizer;

   /**
    * Relevant to both Processes
    * */

   // Affine transform from camera to local mm tray coordinates.
   Eigen::Affine3d create_camera_to_local_transform(const std::string &coordinate_config_prefix);

   // TODO -- push to eigen util class
   void load_eigen_matrix(Eigen::Matrix3Xd &matrix, const std::string &section);

   /**
   *  Properly inits the visualization windows for the camera session.
   **/
   void initialize_visualizers(const std::shared_ptr<depthcam::Session> &session);

   void save_mask(const std::string &id_sequence_step, cv::Mat mask, const std::string &mask_result_dir);

   // TODO Both processes use some version of this, should generalize
   // [[nodiscard]] std::vector<cv::Point2i> get_all_lane_center_pixels

   [[nodiscard]] std::shared_ptr<depthcam::pointcloud::LocalPointCloud> generate_local_traycloud(
       const std::shared_ptr<fulfil::depthcam::Session> &session,
       const Eigen::Affine3d& camera_to_mm_transform,
       float max_height_relative_to_tray_plane = -0.3);

   [[nodiscard]] std::shared_ptr<depthcam::pointcloud::LocalPointCloud> generate_local_traycloud(
       const std::shared_ptr<fulfil::depthcam::Session> &session,
       const Eigen::Affine3d& camera_to_mm_transform,
       float container_width,
       float max_height_relative_to_tray_plane,
       float center_x, float center_y, float min_height_relative_to_tray_plane=0);


   [[nodiscard]] std::experimental::filesystem::path make_save_data_path(const std::string &id_sequence_step,
       const std::string &request_result_dir) const;

   [[nodiscard]] std::vector<std::vector<cv::Point2i>>
       get_width_boundaries(Eigen::Matrix3Xd lane_center_coordinates,
           depthcam::PixelPointConverter local_pix2pt,
           double max_item_width) const;

   void smooth_selection(cv::Mat &selection, bool dilate_first);

   cv::Mat make_tongue_color_selection(cv::Mat rgb_selection);

   std::tuple<std::vector<cv::Point2i>, std::vector<bool>> get_tongue_detections(
       const std::shared_ptr<fulfil::depthcam::Session> &session,
       tray::Tray &current_tray,
       const request_from_vlsg::TrayRequest &tray_vlsg_request,
       std::vector<tray::TrayLane> tray_lanes,
       cv::Mat tongue_color_mask);

   // Above calls below
   std::vector<bool> validate_tongues_in_lane_on_tray(const cv::Mat &tongue_color_mask,
       const std::vector<tray::TrayLane> &tray_lanes,
       const depthcam::PixelPointConverter &local_pix2pt,
       const Eigen::Matrix3Xd &lane_center_coordinates,
       float max_item_width,
       const std::string &pkid);

   // Above calls below
   bool check_roi_for_tongue(const cv::Mat &tongue_color_mask,
       const std::vector<cv::Point2i> &roi_vertices,
       bool has_tongue,
       double masking_bead_cutoff);


   template <typename MaxOp>
   [[nodiscard]] std::array<float, 3> get_matrix_maximum(const Eigen::Matrix3Xd& local_data,
       MaxOp max_op) {
     if  (local_data.cols() <= 0) {
       return {0,0,0};
     }
     Eigen::Index max_index = max_op(local_data); // TODO should be able to elim no?
     if  (max_index < 0) {
       return {0,0,0};
     }
     auto pt = local_data.col(max_index).cast<float>();
     return {pt.x(), pt.y(), pt.z()};
   }

   cv::Mat mark_distance_thresholds(const cv::Mat& depth_mat, float safe_depth_z, bool safe_distance);



   /**
    * Relevant to TVR only
    * */



   // TODO generalize and consolidate
   [[nodiscard]] std::vector<cv::Point2i>
       get_all_lane_center_pixels(fulfil::dispense::tray::Tray &current_tray,
           const depthcam::PixelPointConverter &local_pix2pt,
           float center_line_height=0) const;

   [[nodiscard]] std::array<float, 3> get_max_height_in_tray(
       const std::shared_ptr<depthcam::pointcloud::LocalPointCloud> &tray_point_cloud);

   //cv::Point2i
     //log_max_height_in_tray(const PixelPointConverter &local_pix2pt, std::array<float, 3> max_pt, float expected_max);



   /**
    * Relevant to FED only
    * */

   FEDParams get_fed_params(const fulfil::dispense::tray::TrayLane &current_lane, float lane_center);
   [[nodiscard]] std::array<float, 4> get_height_cut_offs(float current_item_height) const;
   std::array<float, 2> get_width_extents(float current_item_height, float lane_center);

   // TODO generalize and consolidate
   [[nodiscard]] std::vector<cv::Point2i>
       get_all_lane_center_pixels(depthcam::PixelPointConverter local_pix2pt,
           Eigen::Matrix3Xd lane_center_coordinates,
           const std::vector<tray::TrayLane> &tray_lanes,
           int lane_count) const;

   void scan_for_back_item_edge(cv::LineIterator& lane_center_iterator, int &index,
       const depthcam::PixelPointConverter &local_pix2pt,
       const fulfil::dispense::tray::TrayLane &current_lane);

   std::tuple<std::vector<Eigen::Vector3d>, std::vector<cv::Point>> analyze_lane(cv::LineIterator lane_center_iterator,
           const depthcam::PixelPointConverter &local_pix2pt,
           const tray::TrayLane &current_lane,
           FEDParams params);

   [[nodiscard]] double meters_from_tray_front(double y_meters) const;

   [[nodiscard]] results_to_vlsg::LaneItemDistance
       get_first_item_distance_from_coordinate_matrix(
               const std::vector<Eigen::Vector3d> &item_edge_coordinates,
               const tray::TrayLane &lane) const;

   void run_visualizers(cv::Mat mask,
     std::shared_ptr<depthcam::pointcloud::LocalPointCloud> tray_point_cloud,
     const std::string &id_sequence_step,
     std::vector<cv::Point> detections,
     std::array<cv::Point, 2> center_line);



   void add_calibration_pixels(const std::string& config_key);


   /*************************************/



  public:
    TrayAlgorithm(const fulfil::utils::ini::IniSectionReader &tray_config_reader);


    /**
     *  Visualize resulting target for the dispense with a rectangular box of where the item's shadow should be
     **/

    std::tuple<results_to_vlsg::LaneItemDistance, std::vector<tray_count_api_comms::LaneCenterLine>, std::vector<bool>>
        run_tray_algorithm(const std::shared_ptr<fulfil::depthcam::Session>&session,
            const request_from_vlsg::TrayRequest &tray_vlsg_request,
            fulfil::dispense::tray::Tray current_tray);


    /**
     * Generates tray count request information, including lane centers for the given input tray_position (which
     * affects the calibration that is used)
     */
    std::tuple<std::vector<tray_count_api_comms::LaneCenterLine>, std::vector<bool>, float>
        get_pixel_lane_centers_and_tongue_detections(const std::shared_ptr<fulfil::depthcam::Session> &session,
            const request_from_vlsg::TrayRequest &tray_vlsg_request,
            fulfil::dispense::tray::Tray &current_tray);




};

} // namespace fulfil

#endif //FULFIL_DISPENSE_SRC_TRAY_HEIGHT_TRAY_ALGORITHM_H_
