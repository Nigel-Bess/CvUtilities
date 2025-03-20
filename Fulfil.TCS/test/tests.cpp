#include <gtest/gtest.h>
#include "commands/tcs/tcs_perception.h"

using fulfil::dispense::commands::tcs::TCSPerception;

const std::string reqDir = "Fulfil.TCS/data/by-id/";
const std::string testOutDir = "Fulfil.TCS/data/test/";

// Bag Clip states
TEST(BagClip, DetectsAllClosed) {
  auto image = std::make_shared<cv::Mat>(cv::imread("Fulfil.TCS/test/assets/LFB-3.2.jpeg", cv::IMREAD_COLOR));
  auto tcsInference = new TCSPerception();
  auto clipsState = tcsInference->getBagClipStates(image, "LFB-3.2", testOutDir);

  EXPECT_EQ(clipsState->allClipsClosed, true);
  EXPECT_EQ(clipsState->allClipsOpen, false);
  EXPECT_EQ(clipsState->topLeftInference->status, fulfil::dispense::commands::tcs::TCSErrorCodes::Success);
  EXPECT_EQ(clipsState->topLeftInference->isClosed, true);
  EXPECT_EQ(clipsState->topRightInference->isClosed, true);
  EXPECT_EQ(clipsState->bottomLeftInference->isClosed, true);
  EXPECT_EQ(clipsState->bottomRightInference->isClosed, true);

  // TODO: remove this if you see it by now
  std::cout << "1st Bag Clip test passes in CI!";
}
