#include <gtest/gtest.h>
#include "commands/tcs/tcs_perception.h"
#include "commands/tcs/tcs_actions.h"

using fulfil::dispense::commands::tcs::TCSPerception;
using fulfil::dispense::commands::tcs::TCSActions;
using fulfil::utils::Logger;

const std::string reqDir = "Fulfil.TCS/data/by-id/";
const std::string testOutDir = "Fulfil.TCS/data/test/";
const std::string testInDir = "Fulfil.TCS/test/assets/bag_clips/";

static TCSPerception* tcs_inference;
static TCSActions* tcs_actions;

static void setup() {
  if (tcs_inference == nullptr) {
    tcs_inference = new TCSPerception();
    tcs_actions = new TCSActions(tcs_inference);
  }
}

TEST(tcsInference, InitializesStuff) {
  setup();
}

// Bag Clip states

TEST(BagClip, HandlesPrePickupClipDetectionRequest) {
  setup();
  auto image = cv::imread("Fulfil.TCS/assets/baselines/LFP-A_open_white-liner.jpeg", cv::IMREAD_COLOR);
  std::string cmd_id = "0";
  std::shared_ptr<nlohmann::json> request_json = std::make_shared<nlohmann::json>();
  (*request_json)["Primary_Key_ID"] = "123";
  (*request_json)["Tote_Id"] = 1;
  (*request_json)["Facility_Id"] = 1;
  (*request_json)["Bag_Cavity_Index"] = 0;
  (*request_json)["Expected_Bag_Type"] = 1;

  auto result_json = tcs_actions->handle_pre_pick_clip_actuator_request(image, request_json, cmd_id);
  EXPECT_EQ((*result_json)["Error"], fulfil::dispense::commands::tcs::TCSErrorCodes::Success);
  EXPECT_EQ((*result_json)["Tote_Id"], 1);
  EXPECT_EQ((*result_json)["Primary_Key_ID"], "123");
  EXPECT_EQ((*result_json)["Facility_Id"], 1);
  EXPECT_EQ((*result_json)["Clip_Open_States"][0], true);
  EXPECT_EQ((*result_json)["Clip_Open_States"][1], true);
  EXPECT_EQ((*result_json)["Clip_Open_States"][2], true);
  EXPECT_EQ((*result_json)["Clip_Open_States"][3], true);
}
