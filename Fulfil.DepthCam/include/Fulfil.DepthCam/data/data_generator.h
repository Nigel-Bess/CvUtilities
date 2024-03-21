//
// Created by steve on 2/25/20.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * The purpose of this file is to outline the functionality to save
 * the data from a session in the correct format to be used
 * during the instantiation of the MockSession class.
 */
#ifndef FULFIL_DEPTHCAM_DATA_GENERATOR_H
#define FULFIL_DEPTHCAM_DATA_GENERATOR_H

#include <memory>
#include <string>

#include <Fulfil.DepthCam/core/session.h>
#include <json.hpp>

namespace fulfil
{
namespace depthcam
{
namespace data
{
class DataGenerator
{
 private:
  std::shared_ptr<Session> session;
  std::shared_ptr<std::string> destination_directory;
  std::shared_ptr<nlohmann::json> request_json;
  int frames_per_sample;
  void save_frame_information(std::shared_ptr<std::string> frame_directory);
  void save_color_data(std::shared_ptr<std::string> filename);
  void save_aligned_depth_data(std::shared_ptr<std::string> filename);
  void save_raw_depth_data(std::shared_ptr<std::string> filename);
  void save_point_cloud(std::shared_ptr<std::string> filename);


 public:
  /**
   * Constructor
   * @param session the session which will be used to generate the data to save.
   */
  DataGenerator(std::shared_ptr<Session> session,
                std::shared_ptr<std::string> image_path_out,
                std::shared_ptr<std::string> path,
                std::shared_ptr<nlohmann::json> request_json = std::make_shared<nlohmann::json>());

    /**
   * Constructor
   * @param session the session which will be used to generate the data to save.
   */
    DataGenerator(std::shared_ptr<Session> session,
                  std::shared_ptr<std::string> image_path_out,
                  std::shared_ptr<nlohmann::json> request_json = std::make_shared<nlohmann::json>());

  /**
   * Saves the given number of samples from the stored session in the destination directory
   * @param destination_directory the directory where the data from the samples will be stored
   * (this directory should be one that does not exist already).
   * @param num_samples the number of samples that will be taken.
   */
  void save_data(std::shared_ptr<std::string> file_prefix);
  void save_data(const std::string& file_prefix);

  void save_data();

  /**
   *
   * Saves reduces size frames to local disk for duration seconds at frequency fps
   *
   */
  void save_many_frames(int seconds, int fps);
};
}  // namespace mocks
}  // namespace core
}  // namespace fulfil


#endif