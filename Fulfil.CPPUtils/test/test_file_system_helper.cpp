#include <gtest/gtest.h>
#include <Fulfil.CPPUtils/file_system_util.h>

using fulfil::utils::FileSystemUtil;

TEST(fileSystemHelperTests, testDirectoryExists)
{
    ASSERT_TRUE(FileSystemUtil::directory_exists("../../test/test_files"));
    ASSERT_FALSE(FileSystemUtil::directory_exists("../../test/obviously_not_a_directory"));
}

TEST(fileSystemHelperTests, testCreateDirectory)
{
    ASSERT_FALSE(FileSystemUtil::directory_exists("../../test/tmp_dir"));
    FileSystemUtil::create_directory("../../test/tmp_dir");
    ASSERT_TRUE(FileSystemUtil::directory_exists("../../test/tmp_dir"));
    system("rm -r ../../test/tmp_dir");
    ASSERT_FALSE(FileSystemUtil::directory_exists("../../test/tmp_dir"));
}

TEST(fileSystemHelperTests, testFileExists)
{
    ASSERT_TRUE(FileSystemUtil::file_exists("../../test/test_files/example.txt"));
    ASSERT_FALSE(FileSystemUtil::file_exists("../../test/test_files/not_a_file.txt"));
}

TEST(fileSystemHelperTests, testWriteToFile)
{
    ASSERT_FALSE(FileSystemUtil::file_exists("../../test/test_files/output.txt"));
    FileSystemUtil::write_to_file("this is sample output.", strlen("this is sample output."), "../../test/test_files/output.txt");
    ASSERT_TRUE(FileSystemUtil::file_exists("../../test/test_files/output.txt"));
    ASSERT_EQ(std::string("this is sample output."), *FileSystemUtil::get_string_from_file("../../test/test_files/output.txt"));
    system("rm ../../test/test_files/output.txt");
    ASSERT_FALSE(FileSystemUtil::file_exists("../test/test_files/output.txt"));
}

TEST(fileSystemHelperTests, testGetStringFromFile)
{
    ASSERT_EQ(std::string("this is a test file."), *FileSystemUtil::get_string_from_file("../../test/test_files/example.txt"));
}

TEST(fileSystemHelperTests, testGetFilesInDirectory)
{
    auto files = FileSystemUtil::get_files_in_directory(std::make_shared<std::string>("../../test/test_files"));
    ASSERT_EQ(1, files->size());
    ASSERT_EQ(std::string("example.txt"), *files->at(0));
}

TEST(fileSystemHelperTests, testEncodeDecodeLongFlags)
{
  std::shared_ptr<std::vector<bool>> flags = std::make_shared<std::vector<bool>>();
  flags->push_back(true);
  flags->push_back(false);
  flags->push_back(false);
  flags->push_back(true);
  flags->push_back(false);
  FileSystemUtil::write_flags_to_file(flags, std::make_shared<std::string>("tmp"));
  std::shared_ptr<std::vector<bool>> read_flags = FileSystemUtil::read_flags_from_file(std::make_shared<std::string>("tmp"));
  system("rm tmp");
  ASSERT_EQ(read_flags->size(), flags->size());
  for(int i = 0; i < flags->size(); i++)
  {
    ASSERT_EQ(read_flags->at(i), flags->at(i));
  }
}

TEST(fileSystemHelperTests, testEncodingDecodingEmptyFlags)
{
  std::shared_ptr<std::vector<bool>> flags = std::make_shared<std::vector<bool>>();
  FileSystemUtil::write_flags_to_file(flags, std::make_shared<std::string>("tmp"));
  std::shared_ptr<std::vector<bool>> read_flags = FileSystemUtil::read_flags_from_file(std::make_shared<std::string>("tmp"));
  system("rm tmp");
  ASSERT_EQ(read_flags->size(), flags->size());
}

TEST(fileSystemHelperTests, testEncodeDecodeOneFlag)
{
  std::shared_ptr<std::vector<bool>> flags = std::make_shared<std::vector<bool>>();
  flags->push_back(true);
  FileSystemUtil::write_flags_to_file(flags, std::make_shared<std::string>("tmp"));
  std::shared_ptr<std::vector<bool>> read_flags = FileSystemUtil::read_flags_from_file(std::make_shared<std::string>("tmp"));

  system("rm tmp");

  ASSERT_EQ(read_flags->size(), 1);
  ASSERT_TRUE(read_flags->at(0));

  std::shared_ptr<std::vector<bool>> false_flags = std::make_shared<std::vector<bool>>();
  false_flags->push_back(false);
  FileSystemUtil::write_flags_to_file(false_flags, std::make_shared<std::string>("tmp"));
  std::shared_ptr<std::vector<bool>> read_false_flags =
      FileSystemUtil::read_flags_from_file(std::make_shared<std::string>("tmp"));

  system("rm tmp");

  ASSERT_EQ(read_false_flags->size(), 1);
  ASSERT_FALSE(read_false_flags->at(0));
}
