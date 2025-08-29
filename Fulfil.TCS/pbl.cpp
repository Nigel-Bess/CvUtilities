#include <iostream>
#include <string>
#include <bits/stdc++.h>
using namespace std::literals::string_literals;
#include "vmb-manager.h"

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

int main()
{
    auto facility_raw = getenv("FACILITY_IDENTIFIER");
    std::string facility = std::string(facility_raw);
    if (facility.empty()) {
        printf("FACILITY_IDENTIFIER not set, exiting");
        exit(1);
    }
    // printf out "Starting " concatenated with facility
    printf("Starting PBL Vision API at %s\n", facility.c_str());

    auto camera_csv_raw = getenv("CAMERAS");
    std::string camera_csv = std::string(camera_csv_raw);
    if (facility.empty()) {
        printf("CAMERAS not set, exiting\n");
        exit(1);
    }
    // Split camera_csv on comma to get camera info pairs
    std::vector<std::string> camera_info_pairs = split(camera_csv, ',');
    // Split each camera_info_pair on colon to get the index / serial number string pairs
    std::map<int, std::string> bays;
    for (auto camera_info_pair : camera_info_pairs) {
        std::vector<std::string> camera_info = split(camera_info_pair, ':');
        if (camera_info.size() != 2) {
            printf("Invalid camera info pair: %s\n", camera_info_pair.c_str());
        }
        bays.emplace(std::stoi(camera_info[0]), camera_info[1]); // index, serial number
        printf("Registered %s -> %s\n", camera_info[0].c_str(), camera_info[1].c_str());
    }

    auto log = fulfil::utils::Logger::Instance();
    auto man = new VmbManager(bays, log);
}