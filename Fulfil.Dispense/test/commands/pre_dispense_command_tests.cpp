#include "../../commands/pre_dispense/pre_dispense.hpp"
#include "../../commands/pre_dispense/pre_dispense_details.hpp"
#include "spy_depth_cam_command_delegate.hpp"
#include "../helpers/test_helper.hpp"
#include "../fixtures.hpp"
#include <gtest/gtest.h>
#include <memory>

TEST(preDispenseCommandTests, testConstruction)
{
    std::shared_ptr<PreDispenseDetails> pre_dispense_details = Fixtures::pre_dispense_details();
    std::shared_ptr<PreDispense> pre_dispense_command = std::make_shared<PreDispense>(Fixtures::example_id(),
            pre_dispense_details);
    std::shared_ptr<SpyDepthCamCommandDelegate> delegate = std::make_shared<SpyDepthCamCommandDelegate>();
    pre_dispense_command->delegate = delegate->weak_from_this();
    pre_dispense_command->execute();

    EXPECT_STREQ(*pre_dispense_command->command_id, "000000000012");
    EXPECT_EQ(delegate->pre_dispense_details, pre_dispense_details);
}