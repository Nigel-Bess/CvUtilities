/**
 * The purpose of this file is to outline the functionality to save
 * the data from a session in the correct format to be used
 * during the instantiation of the MockSession class.
 */
#ifndef FULFIL_DEPTHCAM_MOCK_SESSION_GENERATOR_H
#define FULFIL_DEPTHCAM_MOCK_SESSION_GENERATOR_H

#include <memory>
#include <string>

#include <Fulfil.DepthCam/core/session.h>

namespace fulfil
{
namespace depthcam
{
namespace mocks
{
class MockSessionGenerator
{
 private:
  std::shared_ptr<Session> session;
  int frames_per_sample;
  void save_frame_information(std::shared_ptr<std::string> frame_directory);
  void save_color_data(std::shared_ptr<std::string> filename);
  void save_depth_data(std::shared_ptr<std::string> filename);
  void save_point_cloud(std::shared_ptr<std::string> filename);
 public:
  /**
   * Constructor
   * @param session the session which will be used to generate the data to save.
   * @param frames_per_sample the numbers of frames stored with each sample taken.
   */
  MockSessionGenerator(std::shared_ptr<Session> session, int frames_per_sample);
  /**
   * Saves the given number of samples from the stored session in the destination directory
   * @param destination_directory the directory where the data from the samples will be stored
   * (this directory should be one that does not exist already).
   * @param num_samples the number of samples that will be taken.
   */
  void save_samples(std::shared_ptr<std::string> destination_directory,
      int num_samples);
};
}  // namespace mocks
}  // namespace core
}  // namespace fulfil


#endif