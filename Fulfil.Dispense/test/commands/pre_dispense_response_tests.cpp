#include "../../commands/pre_dispense/pre_dispense_response.hpp"
#include "../helpers/test_helper.hpp"
#include "../../commands/json.hpp"
#include <cstring>
#include <memory>
#include <gtest/gtest.h>

TEST(preDispenseResponseTests, testFailure)
{
    std::shared_ptr<PreDispenseResponse> pre_dispense_response = std::make_shared<PreDispenseResponse>(TestHelper::string_from_const("000000000012"));

    EXPECT_EQ(strcmp(*pre_dispense_response->get_command_id(), "000000000012"),0);

    EXPECT_EQ(strcmp("{\"Error\":1}", *pre_dispense_response->get_payload()), 0);

    std::shared_ptr<nlohmann::json> expected_json = std::make_shared<nlohmann::json>();
    (*expected_json)["Error"] = 1;
    std::string expected_json_string = expected_json->dump();
    std::shared_ptr<char*> expected_result = TestHelper::string_from_const(expected_json_string.c_str());
    EXPECT_EQ(strlen(*expected_result) + 1, pre_dispense_response->get_payload_size());
}

TEST(preDispenseResponseTests, testSuccess)
{
    std::shared_ptr<PreDispenseResponse> pre_dispense_response = std::make_shared<PreDispenseResponse>(
            TestHelper::string_from_const("000000000012"), 0.1, 0.2, 0.3);

    EXPECT_EQ(strcmp(*pre_dispense_response->get_command_id(), "000000000012"), 0);

    EXPECT_STREQ("{\"Error\":0,\"x\":100,\"y\":200,\"z\":300}", *pre_dispense_response->get_payload());
    std::shared_ptr<nlohmann::json> expected_json = std::make_shared<nlohmann::json>();
    (*expected_json)["Error"] = 0;
    (*expected_json)["X"] = 100;
    (*expected_json)["Y"] = 200;
    (*expected_json)["Z"] = 300;
    std::string expected_json_string = expected_json->dump();
    std::shared_ptr<char*> expected_result = TestHelper::string_from_const(expected_json_string.c_str());
    EXPECT_EQ(strlen(*expected_result) + 1, pre_dispense_response->get_payload_size());
}