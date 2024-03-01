#include "../../commands/error_response.hpp"
#include "../helpers/test_helper.hpp"
#include "../commands/json.hpp"
#include <memory>
#include <gtest/gtest.h>

TEST(errorResponseTests, testNoId)
{
    std::shared_ptr<ErrorResponse> error_response = std::make_shared<ErrorResponse>();

    EXPECT_EQ(nullptr, error_response->get_command_id());

    EXPECT_EQ(strcmp("{\"Error\":1}", *error_response->get_payload()), 0);

    //Actually creating the json object because it uses utf-8 encoding instead of ascii which the system strings use.
    std::shared_ptr<nlohmann::json> expected_json = std::make_shared<nlohmann::json>();
    (*expected_json)["Error"] = 1;
    std::string expected_json_string = expected_json->dump();
    std::shared_ptr<char*> expected_result = TestHelper::string_from_const(expected_json_string.c_str());
    EXPECT_EQ(error_response->get_payload_size(), strlen(*expected_result) + 1);
}

TEST(errorResponseTests, testWithId)
{
    std::shared_ptr<ErrorResponse> error_response = std::make_shared<ErrorResponse>(TestHelper::string_from_const("000000000012"));

    EXPECT_EQ(strcmp("000000000012", *error_response->get_command_id()), 0);

    EXPECT_EQ(strcmp("{\"Error\":1}", *error_response->get_payload()), 0);

    //Actually creating the json object because it uses utf-8 encoding instead of ascii which the system strings use.
    std::shared_ptr<nlohmann::json> expected_json = std::make_shared<nlohmann::json>();
    (*expected_json)["Error"] = 1;
    std::string expected_json_string = expected_json->dump();
    std::shared_ptr<char*> expected_result = TestHelper::string_from_const(expected_json_string.c_str());

    EXPECT_EQ(error_response->get_payload_size(), strlen(*expected_result) + 1);
}