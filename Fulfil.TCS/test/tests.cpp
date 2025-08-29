#include <gtest/gtest.h>
#include "commands/tcs/tcs_perception.h"

using fulfil::dispense::commands::tcs::TCSPerception;
using fulfil::dispense::commands::tcs::PBLBaguetteOrientation;
using fulfil::dispense::commands::tcs::BagType;
using fulfil::utils::Logger;

const std::string reqDir = "Fulfil.TCS/data/by-id/";
const std::string testOutDir = "Fulfil.TCS/data/test/";
const std::string testInDir = "Fulfil.TCS/test/assets/bag_clips/";

// Bag Clip states

TEST(BagClip, DetectsAllOpen) {
  auto image = std::make_shared<cv::Mat>(cv::imread("Fulfil.TCS/assets/baselines/LFP-A_open_white-liner.jpeg", cv::IMREAD_COLOR));
  auto tcsInference = new TCSPerception();
  auto clipsState = tcsInference->getBagClipStates(image, "LFP", "0", testOutDir);

  EXPECT_EQ(clipsState->top_left_inference->status, fulfil::dispense::commands::tcs::TCSErrorCodes::Success);
  EXPECT_EQ(clipsState->all_clips_closed, false);
  EXPECT_EQ(clipsState->top_left_inference->is_closed, false);
  EXPECT_EQ(clipsState->top_right_inference->is_closed, false);
  EXPECT_EQ(clipsState->bottom_left_inference->is_closed, false);
  EXPECT_EQ(clipsState->bottom_right_inference->is_closed, false);
  EXPECT_EQ(clipsState->all_clips_open, true);
}

TEST(BagClip, DetectsAllClosedA) {
  auto image = std::make_shared<cv::Mat>(cv::imread("Fulfil.TCS/assets/baselines/LFP-A_closed_white-liner.jpeg", cv::IMREAD_COLOR));
  auto tcsInference = new TCSPerception();

  auto clipsState = tcsInference->getBagClipStates(image, "LFP", "0", testOutDir);

  EXPECT_EQ(clipsState->top_left_inference->status, fulfil::dispense::commands::tcs::TCSErrorCodes::Success);
  EXPECT_EQ(clipsState->all_clips_closed, true);
  EXPECT_EQ(clipsState->all_clips_open, false);
  EXPECT_EQ(clipsState->top_left_inference->is_closed, true);
  EXPECT_EQ(clipsState->top_right_inference->is_closed, true);
  EXPECT_EQ(clipsState->bottom_left_inference->is_closed, true);
  EXPECT_EQ(clipsState->bottom_right_inference->is_closed, true);
}

TEST(BagClip, DetectsAllClosedB) {
  auto image = std::make_shared<cv::Mat>(cv::imread("Fulfil.TCS/assets/baselines/LFP-B_closed_white-liner.jpeg", cv::IMREAD_COLOR));
  auto tcsInference = new TCSPerception();

  auto clipsState = tcsInference->getBagClipStates(image, "LFP", "0", testOutDir);

  EXPECT_EQ(clipsState->top_left_inference->status, fulfil::dispense::commands::tcs::TCSErrorCodes::Success);
  EXPECT_EQ(clipsState->all_clips_closed, true);
  EXPECT_EQ(clipsState->all_clips_open, false);
  EXPECT_EQ(clipsState->top_left_inference->is_closed, true);
  EXPECT_EQ(clipsState->top_right_inference->is_closed, true);
  EXPECT_EQ(clipsState->bottom_left_inference->is_closed, true);
  EXPECT_EQ(clipsState->bottom_right_inference->is_closed, true);
}

TEST(BagClip, ThrowsLowConfidence) {
  auto image = std::make_shared<cv::Mat>(cv::imread("Fulfil.TCS/test/assets/covered_clips.jpeg", cv::IMREAD_COLOR));
  auto tcsInference = new TCSPerception();
  auto clipsState = tcsInference->getBagClipStates(image, "LFP", "0", testOutDir);

  EXPECT_EQ(clipsState->top_left_inference->status, fulfil::dispense::commands::tcs::TCSErrorCodes::LowConfidenceError);
  EXPECT_EQ(clipsState->all_clips_closed, false);
  EXPECT_EQ(clipsState->all_clips_open, false);
  EXPECT_EQ(clipsState->top_left_inference->is_closed, false);
  EXPECT_EQ(clipsState->top_right_inference->is_closed, false);
  EXPECT_EQ(clipsState->bottom_left_inference->is_closed, false);
  EXPECT_EQ(clipsState->bottom_right_inference->is_closed, false);
}

TEST(BagClip, DetectsAllClosedHellMode) {
  auto image = std::make_shared<cv::Mat>(cv::imread("Fulfil.TCS/test/assets/hell.jpeg", cv::IMREAD_COLOR));
  auto tcsInference = new TCSPerception();

  auto clipsState = tcsInference->getBagClipStates(image, "LFP", "0", testOutDir);

  EXPECT_EQ(clipsState->top_left_inference->status, fulfil::dispense::commands::tcs::TCSErrorCodes::Success);
  EXPECT_EQ(clipsState->all_clips_closed, true);
  EXPECT_EQ(clipsState->all_clips_open, false);
  EXPECT_EQ(clipsState->top_left_inference->is_closed, true);
  EXPECT_EQ(clipsState->top_right_inference->is_closed, true);
  EXPECT_EQ(clipsState->bottom_left_inference->is_closed, true);
  EXPECT_EQ(clipsState->bottom_right_inference->is_closed, true);
}

//Bag Orientation - FRONT_RIGHT_OPENING (Similar to baseline1)

TEST(BagOrientationTest, DetectBagOrientationFrontRight) {
	auto image = std::make_shared<cv::Mat>(cv::imread("Fulfil.TCS/test/assets/Bag-front-right.jpeg", cv::IMREAD_COLOR));
	auto tcsInference = new TCSPerception();
	auto tcsBagOrientation = tcsInference->getBagOrientation(image, testOutDir);

	EXPECT_EQ(tcsBagOrientation->bagOrientation, PBLBaguetteOrientation::FRONT_RIGHT_OPENING);
	EXPECT_EQ(tcsBagOrientation->bagType, BagType::AMBIENT);
	EXPECT_EQ(tcsBagOrientation->bagProblem, false);
}

//Bag Orientation - BACK_LEFT_OPENING (Similar to baseline2)
TEST(BagOrientationTest, DetectBagOrientationBackLeft) {
	auto image = std::make_shared<cv::Mat>(cv::imread("Fulfil.TCS/test/assets/Bag-back-left.jpeg", cv::IMREAD_COLOR));
	auto tcsInference = new TCSPerception();
	auto tcsBagOrientation = tcsInference->getBagOrientation(image, testOutDir);

	EXPECT_EQ(tcsBagOrientation->bagOrientation, PBLBaguetteOrientation::BACK_LEFT_OPENING);
	EXPECT_EQ(tcsBagOrientation->bagType, BagType::AMBIENT);
	EXPECT_EQ(tcsBagOrientation->bagProblem, false);
	std::cerr << "DetectBagOrientationBackLeft test passes in CI!";
}

//Bag Orientation - BACK_RIGHT_OPENING
TEST(BagOrientationTest, DetectBagOrientationBackRight) {
	auto image = std::make_shared<cv::Mat>(cv::imread("Fulfil.TCS/test/assets/Bag-back-right.jpeg", cv::IMREAD_COLOR));
	auto tcsInference = new TCSPerception();
	auto tcsBagOrientation = tcsInference->getBagOrientation(image, testOutDir);

	EXPECT_EQ(tcsBagOrientation->bagOrientation, PBLBaguetteOrientation::BACK_RIGHT_OPENING);
	EXPECT_EQ(tcsBagOrientation->bagType, BagType::AMBIENT);
	EXPECT_EQ(tcsBagOrientation->bagProblem, false);
}

//Bag Orientation - FRONT_LEFT_OPENING 
TEST(BagOrientationTest, DetectBagOrientationFrontLeft) {
	auto image = std::make_shared<cv::Mat>(cv::imread("Fulfil.TCS/test/assets/Bag-front-left.jpeg", cv::IMREAD_COLOR));
	auto tcsInference = new TCSPerception();

	auto tcsBagOrientation = tcsInference->getBagOrientation(image, testOutDir);

	EXPECT_EQ(tcsBagOrientation->bagOrientation, PBLBaguetteOrientation::FRONT_LEFT_OPENING);
	EXPECT_EQ(tcsBagOrientation->bagType, BagType::AMBIENT);
	EXPECT_EQ(tcsBagOrientation->bagProblem, false);
}
