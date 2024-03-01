#include <gtest/gtest.h>
#include "fixtures.h"
#include <Fulfil.Dispense/dispense/image_persistence/timestamper.h>
#include <Fulfil.Dispense/dispense/image_persistence/realsense_file_manager.h>
#include <Fulfil.CPPUtils/file_system_util.h>

using fulfil::dispense::imagepersistence::TimeStamper;
using fulfil::dispense::imagepersistence::RealsenseFileManager;
using fulfil::dispense::commands::DropTargetRequest;
using fulfil::utils::FileSystemUtil;

class MockTimestamper : public TimeStamper
{
public:
    std::shared_ptr<std::string> get_timestamp() override
    {
        return std::make_shared<std::string>("timestamp");
    }
};

TEST(imageFileManagerTests, testDropRequestFilename)
{
    RealsenseFileManager manager(0, std::make_shared<std::string>("images"), std::make_shared<MockTimestamper>());
    std::shared_ptr<std::string> filepath = manager.drop_target_filepath(Fixtures::drop_target_request(), RealsenseFileManager::depth);
    EXPECT_EQ(*filepath, std::string("images/Bay0/predrop_000000000012_depth_timestamp.png"));
}

TEST(imageFileManagerTests, testMainImageDirectoryIsCreated)
{
    //Test that it gets created when its not there
    std::shared_ptr<std::string> directory = std::make_shared<std::string>("images");
    FileSystemUtil::remove_directory(directory);
    RealsenseFileManager manager(0, directory, std::make_shared<MockTimestamper>());
    EXPECT_TRUE(FileSystemUtil::directory_exists(directory->c_str()));

    //Test it still works when the directory is there
    RealsenseFileManager manager2(0, directory, std::make_shared<MockTimestamper>());
    EXPECT_TRUE(FileSystemUtil::directory_exists(directory->c_str()));
    FileSystemUtil::remove_directory(directory);
}

TEST(imageFileManagerTests, testBayDirectoriesAreCreated)
{
    for(int i = 0; i < 4; i++)
    {
      std::shared_ptr<std::string> dir_name = std::make_shared<std::string>();
        dir_name->append("images/Bay");
        dir_name->append(std::to_string(i));
        RealsenseFileManager manager(i, std::make_shared<std::string>("images"), std::make_shared<MockTimestamper>());
        EXPECT_TRUE(FileSystemUtil::directory_exists(dir_name->c_str()));
    }
    FileSystemUtil::remove_directory(std::make_shared<std::string>("../saved_images"));
}