#include <gtest/gtest.h>
#include "commands/tcs/tcs_perception.h"

using fulfil::dispense::commands::tcs::TCSPerception;
using fulfil::dispense::commands::tcs::PBLBaguetteOrientation;
using fulfil::dispense::commands::tcs::BagType;

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

//Bag Orientation - FRONT_RIGHT_OPENING (Similar to baseline1)
TEST(BagOrientationTest, DetectBagOrientationFrontRight) {
	auto image = std::make_shared<cv::Mat>(cv::imread("Fulfil.TCS/test/assets/Bag-front-right.jpeg", cv::IMREAD_COLOR));
	auto tcsInference = new TCSPerception();
	auto tcsBagOrientation = tcsInference->getBagOrientation(image, testOutDir);

	EXPECT_EQ(tcsBagOrientation->bagOrientation, PBLBaguetteOrientation::FRONT_RIGHT_OPENING);
	EXPECT_EQ(tcsBagOrientation->bagType, BagType::AMBIENT);
	EXPECT_EQ(tcsBagOrientation->bagProblem, false);
	std::cout << "DetectBagOrientationFrontRight test passes in CI!";
}

//Bag Orientation - BACK_LEFT_OPENING (Similar to baseline2)
TEST(BagOrientationTest, DetectBagOrientationBackLeft) {
	auto image = std::make_shared<cv::Mat>(cv::imread("Fulfil.TCS/test/assets/Bag-back-left.jpeg", cv::IMREAD_COLOR));
	auto tcsInference = new TCSPerception();
	auto tcsBagOrientation = tcsInference->getBagOrientation(image, testOutDir);

	EXPECT_EQ(tcsBagOrientation->bagOrientation, PBLBaguetteOrientation::BACK_LEFT_OPENING);
	EXPECT_EQ(tcsBagOrientation->bagType, BagType::AMBIENT);
	EXPECT_EQ(tcsBagOrientation->bagProblem, false);
	std::cout << "DetectBagOrientationBackLeft test passes in CI!";
}

//Bag Orientation - BACK_RIGHT_OPENING
TEST(BagOrientationTest, DetectBagOrientationBackRight) {
	auto image = std::make_shared<cv::Mat>(cv::imread("Fulfil.TCS/test/assets/Bag-back-right.jpeg", cv::IMREAD_COLOR));
	auto tcsInference = new TCSPerception();
	auto tcsBagOrientation = tcsInference->getBagOrientation(image, testOutDir);

	EXPECT_EQ(tcsBagOrientation->bagOrientation, PBLBaguetteOrientation::BACK_RIGHT_OPENING);
	EXPECT_EQ(tcsBagOrientation->bagType, BagType::AMBIENT);
	EXPECT_EQ(tcsBagOrientation->bagProblem, false);
	std::cout << "DetectBagOrientationBackRight test passes in CI!";
}

//Bag Orientation - FRONT_LEFT_OPENING 
TEST(BagOrientationTest, DetectBagOrientationFrontLeft) {
	auto image = std::make_shared<cv::Mat>(cv::imread("Fulfil.TCS/test/assets/Bag-front-left.jpeg", cv::IMREAD_COLOR));
	auto tcsInference = new TCSPerception();

	auto tcsBagOrientation = tcsInference->getBagOrientation(image, testOutDir);

	EXPECT_EQ(tcsBagOrientation->bagOrientation, PBLBaguetteOrientation::FRONT_LEFT_OPENING);
	EXPECT_EQ(tcsBagOrientation->bagType, BagType::AMBIENT);
	EXPECT_EQ(tcsBagOrientation->bagProblem, false);
	std::cout << "DetectBagOrientationFrontLeft test passes in CI!";
}