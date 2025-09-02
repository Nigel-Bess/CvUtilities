#include <Fulfil.CPPUtils/orbbec/orbbec_manager.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <tuple>
#include <filesystem>
#include <chrono>

struct sigaction orbbec_manager_destructor;

void orbbec_sig_handler(int sig_no){
    printf("Graceful termination requested");
}

std::vector<std::string> split_str(std::string str, char delimiter)
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

std::map<std::string, std::string> get_orbbec_cam_list_from_env() {
    std::map<std::string, std::string> cam_serial_to_name;
    auto camera_csv_raw = getenv("ORBBEC_CAMERAS");
    std::string camera_csv = std::string(camera_csv_raw);
    if (camera_csv.empty()) {
        printf("ORBBEC_CAMERAS not set, exiting\n");
        exit(1);
    }
    // Split camera_csv on comma to get camera info pairs
    auto tuples = split_str(camera_csv, ',');
    for (auto const &tuple : tuples) {
        auto name_serial = split_str(tuple, ':');
        cam_serial_to_name.emplace(name_serial[1], name_serial[0]);
    }
    return cam_serial_to_name;
}

void OrbbecManager::run_manager() {
    logger->Info("=== Orbbec {} starting ===\n", "");

    auto serial_to_name = get_orbbec_cam_list_from_env();

    // Apply the perfect ctrl+C teardown logic to the K8s-friendly SIGTERM signal as well so the entirety of this service will be Kubernetes friendly for the whole lifecycle
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = &orbbec_sig_handler;
    sigaction(SIGINT, &action, &orbbec_manager_destructor);
    sigaction(SIGTERM, &action, &orbbec_manager_destructor);

    try{
        ob::Context ctx;
        logger->Info("Creating Orbbec context");

        ctx.enableNetDeviceEnumeration(true);
        std::shared_ptr<ob::DeviceList> devices = ctx.queryDeviceList();
        logger->Info("Done querying Orbbec device list, saw {}", devices->deviceCount());

        std::shared_ptr<ob::Config> config = std::make_shared<ob::Config>();
        config->enableVideoStream(OB_STREAM_COLOR);

        for (int i = 0; i < devices->deviceCount(); i++) {
            auto device = devices->getDevice(i);
            // Get device info to extract serial number
            auto info = device->getDeviceInfo();
            std::string serial_number = info->serialNumber();
            // If there's a serial_to_name match, register
            if (serial_to_name.count(serial_number) > 0) {
                auto cam_name = serial_to_name[serial_number];
                logger->Info("Connecting to {}, SN: {}", cam_name, serial_number);
                device = devices->getDeviceBySN(serial_number.c_str());
                name_to_cam[cam_name] = std::make_shared<OrbbecCamera>(cam_name, device, logger);
            } else {
                logger->Info("Found Orbbec not registered by env vars, SN: ", serial_number);
            }
        }
        if (name_to_cam.size() < serial_to_name.size()) {
            logger->Error("Only found {} / {} Orbbec cameras! Exiting...", name_to_cam.size(), serial_to_name.size());
            exit(1);
        }
        logger->Info("OrbbecManager startup successful");
    }
    catch (std::exception &e) {
        logger->Error("Exception starting Orbbec SDK: {}", e.what());
        exit(1);
    }
}

void OrbbecManager::start_manager(){
    std::thread(&OrbbecManager::run_manager, this).detach();
}
