#include "commands/bag_release/repack_perception.h"
#include <filesystem>

using fulfil::utils::Logger;
using fulfil::dispense::commands::RepackPerception;

const auto reqDir = "Fulfil.AlliedVision/data/by-id/";
const auto testOutDir = "Fulfil.AlliedVision/data/test/";

void setupRequestIdFiles(std::string requestId) {
    if (!std::filesystem::exists(reqDir)) {
        Logger::Instance()->Info("Need to download Repack dataset first! See README.md");
        exit(1);
    }

    // Prep test output dir
    auto test_dir = testOutDir + requestId + "/";
    if (!std::filesystem::exists(test_dir)) {
        std::filesystem::create_directories(test_dir);
        Logger::Instance()->Info("Created " + test_dir);
    }

    auto request_dir = reqDir + requestId + "/";
    if (!std::filesystem::exists(request_dir)) {
        throw std::invalid_argument("Request ID does not exist in data/by-id/" + requestId);
    }
}

/*
* Test the Repack Cabinet Empty Bot Detection Algorithm
*/
void testRepackPerception(std::string requestId) {
    if (requestId == "all") {
        // Iterate over all directories in testOutDir and recursively run test
        auto entries = std::filesystem::directory_iterator{reqDir};
        for (const auto &entry: entries) {
            auto path = entry.path().string();
            auto basepath = path.substr(path.find_last_of("/") + 1);
            auto next_requestId = basepath.substr(0, basepath.find("."));

            // Clearing test/data if it exists for next run
            auto reqTestDir = testOutDir + next_requestId + "/";
            if (std::filesystem::exists(reqTestDir + "result.json")) {
                std::filesystem::remove_all(reqTestDir);
            }

            testRepackPerception(next_requestId);
        }
        return;
    }

    // Maybe this is too much an assumption but eh for now
    setupRequestIdFiles(requestId);
    auto requestImg = reqDir + requestId + "/color_image.jpeg";
    // Copy requestImg file to test request dir
    std::string testCopy = testOutDir + requestId + "/color_image.jpeg";

    if (!std::filesystem::exists(testCopy)) {
        std::filesystem::copy(requestImg, testCopy);
    }

    std::string lfb_generation = "LFB-3.2";
    Logger::Instance()->Info("Opening " + testCopy);
    auto image = std::make_shared<cv::Mat>(cv::imread(testCopy, cv::IMREAD_COLOR));
    RepackPerception repack_percep(std::make_shared<std::string>(lfb_generation));
    auto isReady = repack_percep.is_bot_ready_for_release(image, "test", "test1", 0, testOutDir + requestId + "/");

    Logger::Instance()->Info("Img " + testCopy + " is_bag_empty: " + (isReady ? "true" : "false"));
}

/**
 * Test only marker detection
 */
void test_marker_detection(std::string requestId) {
    setupRequestIdFiles(requestId);
    auto aruco = RepackPerception::getRepackArucoTransforms();

    auto requestImg = reqDir + requestId + "/color_image.jpeg";
    Logger::Instance()->Info("Opening " + requestImg);
    auto image = cv::imread(requestImg, cv::IMREAD_COLOR);
    auto result = (*aruco->findImgHomographyFromMarkers(image, testOutDir))[0];
    Logger::Instance()->Info("Got homogggg ");
    auto scaled = aruco->applyHomographyToImg(image, result);
    cv::imwrite(testOutDir + requestId + "/corrected.jpeg", scaled);

    Logger::Instance()->Info("Img {} has {} markers", requestImg, result->max_markers_seen);
}

int main(int argc, char** argv) {
    std::string action = argv[1];
    std::string requestId = argv[2];
    Logger::Instance()->Info("Taking action {} against request {}", action, requestId);

    if (action == "test") {
        testRepackPerception(requestId);
    } else if (action == "mark") {
        test_marker_detection(requestId);
    }

    Logger::Instance()->Info(action + " done!");
	return 0;
}
