#include "../../commands/nop/nop_response.hpp"
#include "../helpers/test_helper.hpp"
#include <memory>
#include <gtest/gtest.h>

TEST(nopResponseTests, testEverythinIsEmpty)
{
    std::shared_ptr<NopResponse> nop_response = std::make_shared<NopResponse>();

    EXPECT_EQ(0, nop_response->get_payload_size());
}