#include "commands/tcs/tcs_perception.h"
#include <filesystem>

using fulfil::dispense::commands::tcs::TCSPerception;
using fulfil::utils::Logger;

const std::string reqDir = "Fulfil.TCS/data/by-id/";
const std::string testOutDir = "Fulfil.TCS/data/test/";

static auto tcsInference = new TCSPerception();

std::string setupTestFolder(std::string feature, std::string request_id)
{
    auto full_req_dir = reqDir + request_id + "/";
    if (!std::filesystem::exists(full_req_dir))
    {
        Logger::Instance()->Info(full_req_dir + " does not exist!");
        exit(1);
    }

    // Prep test output dir
    std::string test_dir = testOutDir + feature + "/" + request_id + "/";

    // Clearing test/data if it exists for next run
    std::string out_file = test_dir + feature + ".json";
    if (std::filesystem::exists(out_file))
    {
        std::filesystem::remove_all(out_file);
        Logger::Instance()->Info("Deleted stale " + out_file);
    }
    if (!std::filesystem::exists(testOutDir)) 
        std::filesystem::create_directory(testOutDir);
    if (!std::filesystem::exists(testOutDir + feature)) 
        std::filesystem::create_directory(testOutDir + feature);
    if (!std::filesystem::exists(test_dir))
        std::filesystem::create_directory(test_dir);
    Logger::Instance()->Info("copy {} {}", full_req_dir, test_dir);
    std::filesystem::copy(full_req_dir, test_dir, std::filesystem::copy_options::skip_existing | std::filesystem::copy_options::recursive);
    std::cerr << "return " + test_dir;
    return test_dir;
}

// Returns the first dir in base_dir and appends to base_dir for the return
std::string append_first_seen_dir(std::string base_dir) {
    for (const auto &dir : std::filesystem::directory_iterator(base_dir)) {
        return dir.path().string() + "/";
    }
    throw std::runtime_error("No dir in " + base_dir);
}

/*
 * Test some TCS Detection Algorithm
 */
void testTCSBagClipsDetection(std::string request_id)
{
    if (request_id == "all")
    {
        // Iterate over all directories in testOutDir and recursively run test
        auto entries = std::filesystem::directory_iterator{reqDir};
        for (const auto &entry : entries)
        {
            auto path = entry.path().string();
            auto basepath = path.substr(path.find_last_of("/") + 1);
            auto next_request_id = basepath.substr(0, basepath.find("."));

            testTCSBagClipsDetection(next_request_id);
        }
        return;
    }

    // Maybe this is too much an assumption but eh for now
    auto testInDir = setupTestFolder("bag_clips", request_id);
    auto prefix_dir = reqDir + request_id + "/Floor_View_Image/";
    auto requestImg = append_first_seen_dir(prefix_dir) + "color_image.png";
    
    std::string lfb_generation = "LFB-3.2";
    auto image = std::make_shared<cv::Mat>(cv::imread(requestImg, cv::IMREAD_COLOR));
    Logger::Instance()->Info("RUN {}", testInDir);
    auto clipsState = tcsInference->getBagClipStates(image, "LFP", request_id, testInDir);
    
    Logger::Instance()->Info("closed {}", clipsState->all_clips_closed);
}

int main(int argc, char **argv)
{
    std::string feature = argv[1];
    std::string request_id = argv[2];
    Logger::Instance()->Info("Evaluating {} against request {}", feature, request_id);

    if (feature == "bag_clips")
    {
        testTCSBagClipsDetection(request_id);
    }

    Logger::Instance()->Info(feature + " done!");
    return 0;
}
