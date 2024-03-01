#include "../../commands/depth_cam_json_parser.hpp"
#include "../fixtures.hpp"
#include "../../commands/pre_dispense/pre_dispense_details.hpp"

#include <gtest/gtest.h>

TEST(depthCamJsonParserTests, testParsingPreDispense)
{
    std::shared_ptr<DepthCamJsonParser> json_parser = std::make_shared<DepthCamJsonParser>();
    std::shared_ptr<PreDispenseDetails> details = json_parser->parse_pre_dispense(Fixtures::pre_dispense_payload_json(),
            Fixtures::example_id());

    EXPECT_STREQ(*details->request_id, "000000000012");
    EXPECT_EQ(details->item_length,0.189);
    EXPECT_EQ(details->item_width, 0.124);
    EXPECT_EQ(details->item_height, 0.055);
    EXPECT_EQ(details->mass, 0);
    EXPECT_EQ(details->material, 0);
    EXPECT_EQ(details->package, 0);
    EXPECT_EQ(details->shape, 0);
    EXPECT_EQ(details->lane_coordinate, 0.321);
    EXPECT_EQ(details->platform_height, 0);
    EXPECT_EQ(details->bag_side, -1);
    EXPECT_EQ(details->limited_right, 0);
    EXPECT_EQ(details->limited_left, 0);
}