#include <gtest/gtest.h>
#include "commands/tcs/tcs_perception.h"
#include "commands/tcs/tcs_actions.h"

using fulfil::dispense::commands::tcs::TCSPerception;
using fulfil::dispense::commands::tcs::TCSActions;
using fulfil::utils::Logger;

const std::string reqDir = "Fulfil.TCS/data/by-id/";
const std::string testOutDir = "Fulfil.TCS/data/test/";
const std::string testInDir = "Fulfil.TCS/test/assets/bag_clips/";

static TCSPerception* tcsInference;

static TCSPerception* setup() {
  if (tcsInference == nullptr) {
    tcsInference = new TCSPerception();
  }
  return tcsInference;
}

// Bag Clip states

TEST(BagClip, HandlesPrePickupClipDetectionRequest) {
  setup();
  auto image = cv::imread("Fulfil.TCS/assets/baselines/LFP-A_open_white-liner.jpeg", cv::IMREAD_COLOR);
  auto tcs_actions = new TCSActions(tcsInference);
  std::string cmd_id = "0";
  std::string pk_id = "0";
  auto response = tcs_actions->handle_pre_pick_clip_actuator_request(image, pk_id, cmd_id);
  auto raw_data = response->message_data();
  auto result_json = std::make_shared<nlohmann::json>(nlohmann::json::parse(raw_data.c_str()));

  EXPECT_EQ((*result_json)["Error"], fulfil::dispense::commands::tcs::TCSErrorCodes::Success);
  EXPECT_EQ((*result_json)["Tote_Id"], 1);
  EXPECT_EQ((*result_json)["Primary_Key_ID"], "0");
  EXPECT_EQ((*result_json)["Facility_Id"], 0);
  EXPECT_EQ((*result_json)["Clip_Open_States"][0], true);
  EXPECT_EQ((*result_json)["Clip_Open_States"][1], true);
  EXPECT_EQ((*result_json)["Clip_Open_States"][2], true);
  EXPECT_EQ((*result_json)["Clip_Open_States"][3], true);
}
