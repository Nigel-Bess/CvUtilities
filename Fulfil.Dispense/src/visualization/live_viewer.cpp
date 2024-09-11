//
// Created by steve on 6/29/20.
//

#include "Fulfil.Dispense/visualization/live_viewer.h"
#include "Fulfil.Dispense/visualization/make_media.h"
#include <Fulfil.Dispense/recipes/visualization_presets.h>

#include <thread>
#include <Fulfil.DepthCam/point_cloud.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/file_system_util.h>
#include <Fulfil.CPPUtils/timer.h>
#include <experimental/filesystem>

namespace std_filesystem = std::experimental::filesystem;
using fulfil::dispense::visualization::LiveViewer;

namespace ffvis = fulfil::dispense::visualization;
using fulfil::utils::FileSystemUtil;
using fulfil::depthcam::Session;
using fulfil::utils::Logger;
namespace mmf = make_media::frame;


LiveViewer::LiveViewer(std::string input_base_path, std::shared_ptr<depthcam::data::FileSender> gcs_sender) :
                                          base_path{input_base_path}, m_gcs_sender{gcs_sender}
{
  FileSystemUtil::create_nested_directory(base_path);
  this->crop_ROI = fulfil::recipes::visualization::LFB3ImageVisualizationSettings().get_roi();
}


bool m_write_image(cv::Mat image, std_filesystem::path image_path)
{
  if (mmf::is_empty(image))  {
    Logger::Instance()->Debug("Attempt to write image to {} failed due to empty input mat!", image_path.string());
    return false;
  }
  Logger::Instance()->Debug("Writing image at full size to: {}", image_path.string());
  return cv::imwrite(image_path.string(), image);
}


bool LiveViewer::write_image(const cv::Mat& image, std_filesystem::path image_path, double resize_factor)
{
  auto timer = fulfil::utils::timing::Timer("LiveViewer::write_image " + image_path.string() );
  if(resize_factor == 1 || resize_factor == 0) return m_write_image(image, image_path);
  if (mmf::is_empty(image))  {
    Logger::Instance()->Debug("Attempt to write image to {} failed due to empty input mat!", image_path.string());
    return false;
  }//error check
  cv::Mat reduced_image;
  Logger::Instance()->Debug("Writing image reduced by {} to: {}", resize_factor, image_path.string());

  cv::resize(image, reduced_image, cv::Size(), resize_factor, resize_factor);
  return cv::imwrite(image_path.string(), reduced_image);
}


void boarder_visualization(cv::Mat& output_img){
  int pixel_boarder = 4;
  cv::copyMakeBorder(output_img, output_img, pixel_boarder, pixel_boarder, pixel_boarder, pixel_boarder, cv::BORDER_CONSTANT);
}


std::shared_ptr<cv::Mat> make_empty_map()
{
  fulfil::recipes::visualization::LFB3GridSettings map_grid {};
  cv::Mat output_img = cv::Mat(map_grid.m_rows*map_grid.m_resize_factor, map_grid.m_cols*map_grid.m_resize_factor,
    CV_8UC3, cv::Scalar(255, 255, 255));
  boarder_visualization(output_img);
  return std::make_unique<cv::Mat>(output_img);
}


std::shared_ptr<cv::Mat> LiveViewer::get_damage_risk_visualization(std::shared_ptr<cv::Mat> input_img_ptr)
{
  if(mmf::is_empty(input_img_ptr)) {
    Logger::Instance()->Warn("Empty damage risk input given!");
    return  make_empty_map();
  }
  Logger::Instance()->Info("There should be {} cells highlighted in item damage risk", cv::countNonZero(*input_img_ptr));
  auto map_grid = fulfil::recipes::visualization::LFB3GridSettings();
  cv::Mat output_img = cv::Mat(map_grid.m_rows, map_grid.m_cols, CV_8UC3, cv::Scalar(255, 255, 255));

  for(int i = 0; i < map_grid.m_rows; i++) // swap to iterator
  {
    for(int j = 0; j < map_grid.m_cols; j++)
    {
      int value = input_img_ptr->at<uchar>(i, j);

      if (value == 1) { //red
        output_img.at<cv::Vec3b>(i,j)[0] = 0;
        output_img.at<cv::Vec3b>(i,j)[1] = 0;
        output_img.at<cv::Vec3b>(i,j)[2] = 255; // red
      }
      if (value == 2) { // orange
        output_img.at<cv::Vec3b>(i,j)[0] = 0;    //blue
        output_img.at<cv::Vec3b>(i,j)[1] = 165;    //green
        output_img.at<cv::Vec3b>(i,j)[2] = 255;  //red
      }
    }
  }

  cv::resize(output_img, output_img, cv::Size(), map_grid.m_resize_factor, map_grid.m_resize_factor);
  boarder_visualization(output_img);
  return std::make_unique<cv::Mat>(output_img);
}

std::shared_ptr<cv::Mat> LiveViewer::get_item_detection_visualization(std::shared_ptr<cv::Mat> input_img_ptr)
{
  //note: input will be nullptr if no item was detected. In this case, want a blank white visualization
  //note: want to create a copy of any image passed in via pointer so don't change outside this function
  // these must be the same size right? why not use the image ptr?
  if(mmf::is_empty(input_img_ptr)) {
    Logger::Instance()->Warn("Empty item detection input given!");
    return  make_empty_map();
  }
  Logger::Instance()->Info("There should be {} cells highlighted in item detection viz input", cv::countNonZero(*input_img_ptr));



  auto map_grid = fulfil::recipes::visualization::LFB3GridSettings();

  cv::Mat output_img = cv::Mat(map_grid.m_rows, map_grid.m_cols, CV_8UC3, cv::Scalar(255, 255, 255));
  for(int i = 0; i < map_grid.m_rows; i++) // swap to iterator
  {
    for(int j = 0; j < map_grid.m_cols; j++)
    {
      int value = input_img_ptr->at<float>(i, j); //TODO: change output of depth pre/post analysis to int?
      if (value == 1) { // is this one or zero? nothing else?
        output_img.at<cv::Vec3b>(i,j)[0] = 255;    //blue
        output_img.at<cv::Vec3b>(i,j)[1] = 0;
        output_img.at<cv::Vec3b>(i,j)[2] = 0;

      }
    }

  }

  cv::resize(output_img, output_img, cv::Size(), map_grid.m_resize_factor, map_grid.m_resize_factor);
  boarder_visualization(output_img);

  return std::make_unique<cv::Mat>(output_img);
}

std::shared_ptr<cv::Mat> LiveViewer::get_blank_visualization()
{
  return std::make_shared<cv::Mat>(this->crop_ROI.height, this->crop_ROI.width, CV_8UC3, cv::Scalar(255, 255, 255));
}

void LiveViewer::add_message_to_image(cv::Mat &image, std::shared_ptr<std::vector<std::string>> message)
{
  int num_lines = message->size();
  bool is_error = true; // Keep track of if subsequent lines should also be red
  int thickness = 4;
  float font_scale = 1.5;

  auto get_color = [&is_error] (auto line_text) {
    if (line_text == "Target: Success")  { is_error = false; return cv::Scalar(50,150,0); }
    if (is_error) return cv::Scalar(0,0,255);
    if (line_text == "Item in bag: No") { is_error = true; cv::Scalar(0,0,255); }
    if(line_text == "LFR is Rotated") { return cv::Scalar(255,0,0); }
    return  cv::Scalar(0,0,0);
  };

  for(int line = 0; line < num_lines; line++)
  {
    std::string line_text = message->at(line);
    cv::putText(image, line_text, cv::Point(20,75 + (line + 1) * 100), cv::FONT_HERSHEY_DUPLEX , font_scale, get_color(line_text), thickness, cv::LINE_AA);
  }
}


std::tuple<std::string, double> LiveViewer::image_name_and_presets(size_t image_code) {
  const double lfb = 0.6;
  const double tray = 0.5;

  std::array<const char*, VIEWER_IMAGE_TYPE_COUNT> image_name_map
    { "LFB_RGB",           "LFB_Depth",          "LFB_Damage_Risk",    "LFB_Filter",
      "LFB_Target",        "Tray_Result",        "LFB_Pre_Dispense",   "Tray_Pre_Dispense",
      "LFB_Post_Dispense", "Tray_Post_Dispense", "LFB_Item_Detection", "LFB_Markers",
       "Info", "LFB_Floor_View", "Tray_Pre_FED", "Tray_Post_FED", "Tray_Validation"
    };
  std::array<double, VIEWER_IMAGE_TYPE_COUNT> resize_presets {
    lfb,  lfb,   1,   lfb,
    lfb,  tray,  lfb, tray,
    lfb,  tray,  1,   lfb,
    lfb, lfb, tray, tray, tray
  };
  if (image_code > VIEWER_IMAGE_TYPE_COUNT + 1) return {"", 0};
  return {image_name_map[image_code], resize_presets[image_code]};
}

std_filesystem::path name_visualization(std_filesystem::path base_path, const std::string& PKID, const std::string image_name) {
  auto timer = fulfil::utils::timing::Timer("LiveViewer::name_visualization for " + PKID + " type " + image_name);
  std_filesystem::path save_vis_path = base_path / std_filesystem::path{PKID};
  std_filesystem::create_directories(save_vis_path);
  save_vis_path /=  image_name;
  save_vis_path.replace_extension(".jpg");
  return save_vis_path;
}


std_filesystem::path pkid_filename(std::string_view PKID, std::string_view image_name) {
  std_filesystem::path save_vis_path = std_filesystem::path {PKID} / std_filesystem::path {image_name};
  save_vis_path.replace_extension(".jpg");
  return save_vis_path;
}

int LiveViewer::publish(std_filesystem::path pkid_tagged_image) {
  std_filesystem::path local_vis_path = base_path / pkid_tagged_image;
  fulfil::utils::Logger::Instance()->Debug("Sending file {} to {}", local_vis_path.string(), pkid_tagged_image.string());

  // TODO make meaningful return
  return m_gcs_sender->send_file(local_vis_path.string(),
    pkid_tagged_image.string(), true, false);
}

int LiveViewer::publish(size_t image_number, std::string PKID){
  auto [image_name, resize] = image_name_and_presets(image_number);
  auto vis_path =  name_visualization(this->base_path, PKID, image_name);
  auto remote_path = PKID + '/' + vis_path.filename().string();
  fulfil::utils::Logger::Instance()->Debug("Sending file {} to {}", vis_path.string(), remote_path);

  // TODO make meaningful return
  return m_gcs_sender->send_file(vis_path.string(),
    remote_path, true, false);
}


int LiveViewer::update_image(std::shared_ptr<cv::Mat> input_image,
  size_t image_number, std::string PKID, bool publish_now, std::shared_ptr<std::vector<std::string>> message)
{

  auto [image_name, resize] = image_name_and_presets(image_number);
  Logger::Instance()->Debug("LIVE VIEWER: updating image to be written, of type: {}", image_name);
  auto timer = fulfil::utils::timing::Timer("LiveViewer::update_image for " + PKID + " type " + image_name);


  if (mmf::is_empty(input_image)) { return 0 ; }
  cv::Mat updated_image = make_media::frame::copy_matrix(input_image); //clone image so that original in algorithms are not affected by rotation/cropping

  //if visualization is in specified set of visualizations and rotation is set true from LFB_configs, rotate the image
  //                                                                  0     1     2      3     4     5      6     7      8      9      10     11    12    13    14     15     16
  std::array<bool, VIEWER_IMAGE_TYPE_COUNT> images_to_be_rotated = { true, true, false, true, true, false, true, false, true,  false, false, true, true, true, false, false, false };
  std::array<bool, VIEWER_IMAGE_TYPE_COUNT> images_to_be_cropped = { true, true, false, true, true, false, true, false, true,  false, false, true, false, true, false, false, false };
  bool crop = image_number < VIEWER_IMAGE_TYPE_COUNT && images_to_be_cropped[image_number];
  bool rotate = image_number < VIEWER_IMAGE_TYPE_COUNT && images_to_be_rotated[image_number];

  if(crop) updated_image = (updated_image)(this->crop_ROI);
  if(rotate) cv::rotate(updated_image, updated_image, cv::ROTATE_90_CLOCKWISE);


  //add message to image in error cases
  if(message != nullptr) add_message_to_image(updated_image, message);
  auto vis_path =  name_visualization(this->base_path, PKID, image_name);

  this->write_image(updated_image, vis_path, resize);
  if (publish_now) {
    return publish(pkid_filename(PKID, image_name));
  }
  return 0;
}

