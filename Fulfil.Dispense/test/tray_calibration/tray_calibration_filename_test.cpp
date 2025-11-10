#include <gtest/gtest.h>
#include <regex>
#include <filesystem>
#include <Fulfil.CPPUtils/inih/INIReader.h>

std::vector<std::string> extractFilesWithSerialNumbers(const std::string& directoryPath)
{
    std::vector<std::string> filenames;
    std::regex pattern(R"(tray_calibration_data_(\d+)_(?:dispense|hover)\.ini)");

    for (const auto& entry : std::filesystem::directory_iterator(directoryPath))
    {
        if (entry.is_regular_file())
        {
            std::string filename = entry.path().filename().string();
            std::smatch match;

            //Check if filename matches the regex pattern for hover or dispense
            if (std::regex_match(filename, match, pattern))
            {
                filenames.push_back(filename);
            }
        }
    }

    return filenames;
}

std::string extractSerialNumberFromFile(const std::string& filename)
{
    std::regex pattern(R"(tray_calibration_data_(\d+)_(?:dispense|hover)\.ini)");
    std::smatch match;

    if (std::regex_match(filename, match, pattern))
    {
        return match[1]; // Return the captured serial number
    }

    return ""; // Return empty string if no match found
}

TEST(TrayCalibrationFilenameTest, CheckINIFileContainsCorrectSerialNumber)
{
    const std::string configDirectory = std::string(TEST_DATA_DIR "/tray_calibration");

    std::vector<std::string> extractedFilenames = extractFilesWithSerialNumbers(configDirectory);

    for (const auto& filename : extractedFilenames)
    {
        const std::string completeFilePath = configDirectory + "/" + filename;
        std::shared_ptr<INIReader> reader = std::make_shared<INIReader>(completeFilePath, false, true);
        EXPECT_NE(reader, nullptr) << "Failed to read INI file: " << completeFilePath;

        std::string serialNumberInFile = extractSerialNumberFromFile(filename);
        EXPECT_NE(serialNumberInFile, "") << "Failed to extract serial number from filename: " << filename;

        std::string configSectionSubstr = "";
        if (filename.find("hover") != std::string::npos)
            configSectionSubstr = "_hover";

        std::string configSection = serialNumberInFile + configSectionSubstr +"_pixel_locations";
        std::vector<int> pixel_locations_x = reader->GetIntegerVector(configSection, "x");
        EXPECT_EQ(pixel_locations_x.size(), 16);

        std::vector<int> pixel_locations_y = reader->GetIntegerVector(configSection, "y");
        EXPECT_EQ(pixel_locations_y.size(), 16);

        std::vector<int> pixel_locations_depth = reader->GetIntegerVector(configSection, "depth");
        EXPECT_EQ(pixel_locations_depth.size(), 16);

        configSection = serialNumberInFile + configSectionSubstr +"_camera_coordinates";
        std::vector<float> camera_coordinates_x = reader->GetFloatVector(configSection, "x");
        EXPECT_EQ(camera_coordinates_x.size(), 16);

        std::vector<float> camera_coordinates_y = reader->GetFloatVector(configSection, "y");
        EXPECT_EQ(camera_coordinates_y.size(), 16);

        std::vector<float> camera_coordinates_depth = reader->GetFloatVector(configSection, "depth");
        EXPECT_EQ(camera_coordinates_depth.size(), 16);

        configSection = serialNumberInFile + configSectionSubstr + "_tray_coordinates";
        std::vector<float> tray_coordinates_x = reader->GetFloatVector(configSection, "x");
        EXPECT_EQ(tray_coordinates_x.size(), 16);

        std::vector<float> tray_coordinates_y = reader->GetFloatVector(configSection, "y");
        EXPECT_EQ(tray_coordinates_y.size(), 16);

        std::vector<float> tray_coordinates_depth = reader->GetFloatVector(configSection, "depth");
        EXPECT_EQ(tray_coordinates_depth.size(), 16);
    }
}



// main to run the tests above
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
