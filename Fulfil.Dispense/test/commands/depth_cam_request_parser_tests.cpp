#include "../../commands/depth_cam_request_parser.hpp"
#include "../helpers/test_helper.hpp"
#include "../../commands/depth_cam_command_delegate.hpp"
#include "../../commands/depth_cam_request_json_parser.hpp"
#include "../fixtures.hpp"
#include <gtest/gtest.h>
#include <memory>

class SpyWasCalledDepthCamCommandDelegate : public DepthCamCommandDelegate,
        public enable_shared_from_this<SpyWasCalledDepthCamCommandDelegate>
{
public:
    bool handle_pre_dispense_was_called = false;
    std::shared_ptr<PreDispenseDetails> pre_dispense_details;
    bool handle_stop_request_was_called = false;

    std::shared_ptr<DropResult> handle_pre_dispense(std::shared_ptr<PreDispenseDetails> details)
    {
        this->pre_dispense_details = details;
        this->handle_pre_dispense_was_called = true;
    }
    void handle_stop_request(std::shared_ptr<char*> command_id)
    {
        this->handle_stop_request_was_called = true;
    }
};

class SpyDepthCamRequestJsonParser : public DepthCamRequestJsonParser
{
public:
    std::shared_ptr<PreDispenseDetails> example_details;
    bool parse_pre_dispense_was_called = false;
    SpyDepthCamRequestJsonParser()
    {
        this->example_details =  std::make_shared<PreDispenseDetails>(TestHelper::const_to_shared("000000000012"), 0, 0,
                                                                      0, 0, 0, 0, 0, 0,
                                                                      0, 0, 0, 0);
    }
    std::shared_ptr<PreDispenseDetails> parse_pre_dispense(std::shared_ptr<nlohmann::json> request_json,
                                                           std::shared_ptr<char*> command_id)
    {
        this->parse_pre_dispense_was_called = true;
        return this->example_details;
    }
};

TEST(depthCamRequestParserTests, testParsingPreDrop)
{
    std::shared_ptr<SpyDepthCamRequestJsonParser> json_parser = std::make_shared<SpyDepthCamRequestJsonParser>();
    std::shared_ptr<DepthCamRequestParser> request_parser = std::make_shared<DepthCamRequestParser>(json_parser);
    std::shared_ptr<DepthCamCommand<std::shared_ptr<DepthCamResponse>>> command =
            request_parser->parse_payload(Fixtures::pre_dispense_payload(), Fixtures::example_id());
    std::shared_ptr<SpyWasCalledDepthCamCommandDelegate> delegate = std::make_shared<SpyWasCalledDepthCamCommandDelegate>();
    command->delegate = delegate->weak_from_this();
    command->execute();

    EXPECT_STREQ(*command->command_id, "000000000012");
    EXPECT_EQ(command->retry_limit, -1);
    EXPECT_TRUE(json_parser->parse_pre_dispense_was_called);
    EXPECT_EQ(delegate->pre_dispense_details, json_parser->example_details);
    EXPECT_TRUE(delegate->handle_pre_dispense_was_called);
    EXPECT_FALSE(delegate->handle_stop_request_was_called);
}

TEST(depthCamREquestParserTests, testParsingStop)
{
    std::shared_ptr<SpyDepthCamRequestJsonParser> json_parser = std::make_shared<SpyDepthCamRequestJsonParser>();
    std::shared_ptr<DepthCamRequestParser> request_parser = std::make_shared<DepthCamRequestParser>(json_parser);
    std::shared_ptr<DepthCamCommand<std::shared_ptr<DepthCamResponse>>> command =
            request_parser->parse_payload(Fixtures::stop_payload(), Fixtures::example_id());
    std::shared_ptr<SpyWasCalledDepthCamCommandDelegate> delegate = std::make_shared<SpyWasCalledDepthCamCommandDelegate>();
    command->delegate = delegate->weak_from_this();
    command->execute();

    EXPECT_STREQ(*command->command_id, "000000000012");
    EXPECT_EQ(command->retry_limit, 0);
    EXPECT_TRUE(delegate->handle_stop_request_was_called);
    EXPECT_FALSE(delegate->handle_pre_dispense_was_called);
}