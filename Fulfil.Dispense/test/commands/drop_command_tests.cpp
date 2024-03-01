#include "../../commands/drop_command.hpp"
#include <memory>
#include <gtest/gtest.h>

TEST(dropCommandTests, testParsingValidDropCommand)
{
    EXPECT_EQ(DropCommand::nop, DropCommandParser::parse(0));

    EXPECT_EQ(DropCommand::pre_dispense, DropCommandParser::parse(1));

    EXPECT_EQ(DropCommand::post_dispense, DropCommandParser::parse(2));

    EXPECT_EQ(DropCommand::tray_height, DropCommandParser::parse(3));

    EXPECT_EQ(DropCommand::dispense_distance, DropCommandParser::parse(4));

    EXPECT_EQ(DropCommand::stop_call, DropCommandParser::parse(5));
}

TEST(dropCommandTests, testParsingInvalidNegativeCommand)
{
    try
    {
        DropCommandParser::parse(-1);
        EXPECT_EQ(false, true);
    }
    catch (const std::invalid_argument& e)
    {
        EXPECT_STREQ(e.what(), "Invalid Command Type");
    }
}

TEST(dropCommandTests, testParsingInvalidLargeCommand)
{
    try
    {
        DropCommandParser::parse(6);
        EXPECT_EQ(false, true);
    }
    catch (const std::invalid_argument& e)
    {
        EXPECT_STREQ(e.what(), "Invalid Command Type");
    }

}