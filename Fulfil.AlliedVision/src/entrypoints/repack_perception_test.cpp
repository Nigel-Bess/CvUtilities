#include "commands/bag_release/repack_perception.h"
#include <filesystem>

//using namespace fulfil::dispense::commands;
using fulfil::utils::Logger;
using fulfil::dispense::commands::RepackPerception;

void setupRequestIdFiles(std::string request_id, std::string test_out_dir) {
    if (!std::filesystem::exists("Fulfil.AlliedVision/data/by-id")) {
        Logger::Instance()->Info("Need to download Repack dataset first! See README.md");
        exit(1);
    }

    // Prep test output dir
    auto test_dir = test_out_dir + request_id + "/";
    if (!std::filesystem::exists(test_dir)) {
        std::filesystem::create_directories(test_dir);
        Logger::Instance()->Info("Created " + test_dir);
    }

    auto request_dir = "Fulfil.AlliedVision/data/by-id/" + request_id + "/";
    if (!std::filesystem::exists(request_dir)) {
        throw std::invalid_argument("Request ID does not exist in data/by-id/" + request_id);
    }
}

/*
* Test the Repack Cabinet Empty Bot Detection Algorithm
*/
void test_repack_perception(std::string request_img, std::string test_out_dir, std::string lfb_generation) {
    Logger::Instance()->Info("Opening " + request_img);
    auto image = std::make_shared<cv::Mat>(cv::imread(request_img, cv::IMREAD_COLOR));
    RepackPerception repack_percep(std::make_shared<std::string>(lfb_generation));
    auto isReady = repack_percep.is_bot_ready_for_release(image, test_out_dir);

    Logger::Instance()->Info("Img " + request_img + " is_bag_empty: " + (isReady ? "true" : "false"));
}

int main(int argc, char** argv) {
    // Save first cli arg as std::string called request_id
    std::string request_id = argv[1];
    std::string lfb_generation = argv[2];

    setupRequestIdFiles(request_id, "Fulfil.AlliedVision/data/test/");
    test_repack_perception("Fulfil.AlliedVision/data/by-id/" + request_id + "/color_image.jpeg", "Fulfil.AlliedVision/data/test/", lfb_generation);
	return 0;
}
