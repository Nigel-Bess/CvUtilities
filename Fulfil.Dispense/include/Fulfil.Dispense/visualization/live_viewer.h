//
// Created by steve on 6/29/20.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_VISUALIZATION_LIVE_VIEWER_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_VISUALIZATION_LIVE_VIEWER_H_
#include "Fulfil.DepthCam/data/file_sender.h"
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include <Fulfil.DepthCam/core.h>
#include <experimental/filesystem>
#include <memory>
#include <mutex>
#include <opencv2/opencv.hpp>


namespace fulfil::dispense::visualization
{
/**
 * The purpose of this class is to handle live visuals of image algorithm results
 */
 
enum ViewerImageType {
  LFB_RGB, LFB_Depth, LFB_Damage_Risk, LFB_Filter,
  LFB_Target, Tray_Result, LFB_Pre_Dispense, Tray_Pre_Dispense,
  LFB_Post_Dispense, Tray_Post_Dispense, LFB_Item_Detection, LFB_Markers,
  Info
};

struct ABISVisualizationSettings
{
  std::string m_image_name {};
  double resize_factor {};
  cv::Rect ROI_crop{};
  int mapped_code{};
  bool rotate {};
  ABISVisualizationSettings() = default;
};

class LiveViewer
{
 private:
   std::experimental::filesystem::path base_path;
   std::shared_ptr<depthcam::data::FileSender> m_gcs_sender;
   //region for cropping visualizations
   cv::Rect crop_ROI;

   int publish(std::experimental::filesystem::path local_path);

 public:
  /**
   * LiveViewer constructor
   * @param bay - the id of the bay this streamer will display
   */
  LiveViewer(std::string input_base_path, std::shared_ptr<depthcam::data::FileSender> gcs_sender);

  //function for writing image to output file location
  // note: rotate_image allows rotation if it is also set True in LFB_config.ini
  bool write_image(const cv::Mat& image, std::experimental::filesystem::path image_path, double resize_factor);

  //blank visualization used for error + information visualizations
  std::shared_ptr<cv::Mat> get_blank_visualization();
  std::shared_ptr<cv::Mat> get_item_detection_visualization(std::shared_ptr<cv::Mat> input_img_ptr);
  std::shared_ptr<cv::Mat> get_damage_risk_visualization(std::shared_ptr<cv::Mat> input_img_ptr);



  /**
   * Tells the streamer to start streaming, i.e. saving visualization images locally
   */

  void add_message_to_image(cv::Mat& image, std::shared_ptr<std::vector<std::string>> message);

  /**
   * Updates the drop_result_image with most recent algorithm result
   */
  int update_image(std::shared_ptr<cv::Mat> input_image,
    size_t image_number, std::string PKID, bool publish_now=true, std::shared_ptr<std::vector<std::string>> message = nullptr);

  int publish(size_t image_number, std::string PKID);


  /**
   *  Rename certain visualizations if they already exist for the given PKID
   *  This would be true during an LFB Pirouette - where the drop target + markers images will already exist from previous request
   */


};
} // namespace visualization

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_VISUALIZATION_LIVE_VIEWER_H_
