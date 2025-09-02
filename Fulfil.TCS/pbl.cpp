#include <iostream>
#include <string>
#include <bits/stdc++.h>
using namespace std::literals::string_literals;
#include "vmb-manager.h"
#include <chrono>
#include <thread>
#include "libobsensor/ObSensor.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/core/hal/interface.h>

using namespace VmbCPP;
using namespace std;

std::vector<std::string> split(string str, char delimiter)
{
  // Using str in a string stream
    std::stringstream ss(str);
    std::vector<std::string> res;
    std::string token;
    while (getline(ss, token, delimiter)) {
        res.push_back(token);
    }
    return res;
}

void stuff() { 

    /*try {
        cout << "Getting device CPEH552000B6" << endl;
        auto device = ctx.createNetDevice("10.10.10.25", 8090);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        //auto dev1 = devices->getDeviceBySN("CPEH552000B6");
        cout << "Got device!" << endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        auto info = device->getDeviceInfo();
        cout << "Got info!" << endl;
        cout << "  SN: " << info->serialNumber() << endl;
        cout << "  name: " << info->name() << endl;
    }
    catch(ob::Error &e) {
        std::cerr << "function:" << e.getName() << "\nargs:" << e.getArgs() << "\nmessage:" << e.getMessage() << "\ntype:" << e.getExceptionType() << std::endl;
        exit(-1);
    }

    std::shared_ptr<ob::Config> config = std::make_shared<ob::Config>();
    config->enableVideoStream(OB_STREAM_COLOR);
    cout << "Made config" << endl;

    for (int i = 0; i < devices->deviceCount(); i++) {
        cout << "Getting device" << endl;
        auto device = devices->getDevice(i);
        cout << "Got device" << endl;
        auto info = device->getDeviceInfo();
        cout << "Got device info" << endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
        cout << "  SN: " << info->serialNumber() << endl;
        cout << "  name: " << info->name() << endl;

        auto pipe = std::make_shared<ob::Pipeline>(device);
        // Configure which streams to enable or disable for the Pipeline by creating a Config
        cout << "video enabled" << endl;
        // Start the pipeline with config
        pipe->start(config);
        cout << "video started" << endl;
        
        auto frameSet = pipe->waitForFrames(30000);
        cout << "got frame" << endl;
        auto colorFrame = frameSet->colorFrame();
        cout << "got color" << endl;
        colorFrame->data();
        cout << "got color data" << endl;
        auto index = colorFrame->index(); 
        cout << "got index" << endl;
        cv::Mat my_frame = cv::Mat(colorFrame->height(), colorFrame->width(), CV_8UC3, colorFrame->data());
        cout << "created" << endl;
        uchar pixel_value = my_frame.at<uchar>(1, 1);
        cout << "pixel is " + std::to_string(pixel_value) << endl;
        saveColor(colorFrame);
        pipe->stop();
        cout << "stopped" << endl;
    }
    cout << "Orbbec connections healthy" << endl;
    */
}

std::string get_facility() {
    auto facility_raw = getenv("FACILITY_IDENTIFIER");
    std::string facility = std::string(facility_raw);
    if (facility.empty()) {
        printf("FACILITY_IDENTIFIER not set, exiting");
        exit(1);
    }
    return facility;
}

std::vector<std::string> get_vimba_cam_list() {
    auto camera_csv_raw = getenv("VIMBA_CAMERAS");
    std::string camera_csv = std::string(camera_csv_raw);
    if (camera_csv.empty()) {
        printf("VIMBA_CAMERAS not set, exiting\n");
        exit(1);
    }
    // Split camera_csv on comma to get camera info pairs
    return split(camera_csv, ',');
}

int main()
{
    try {
        stuff();
    } catch (...) {
        cout << "Exception caught" << endl;
        return 1;
    }
    auto facility = get_facility();
    // printf out "Starting " concatenated with facility
    printf("Starting PBL Vision API at %s\n", facility.c_str());

    /*std::vector<std::string> orbbec_info_pairs = set_orbbec_cam_list_from_env(); 
    std::vector<std::string> vimba_info_pairs = get_vimba_cam_list(); 
    // Split each camera_info_pair on colon to get the index / serial number string pairs
    std::map<int, std::string> bays;
    for (auto camera_info_pair : vimba_info_pairs) {
        std::vector<std::string> camera_info = split(camera_info_pair, ':');
        if (camera_info.size() != 2) {
            printf("Invalid camera info pair: %s\n", camera_info_pair.c_str());
        }
        bays.emplace(std::stoi(camera_info[0]), camera_info[1]); // index, serial number
        printf("Registered %s -> %s\n", camera_info[0].c_str(), camera_info[1].c_str());
    }

    auto log = fulfil::utils::Logger::Instance();
    auto man = new VmbManager(bays, log);*/
}