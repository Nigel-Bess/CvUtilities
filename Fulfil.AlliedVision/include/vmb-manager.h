#pragma once

#include <json.hpp>
#include "vmb-camera.h"

inline std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        std::cout << "parameter::exec pipe was null" << std::endl;
        return "";
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    printf("parameter::exec [%s] --> [%s]\n", cmd, result.substr(0, result.length() - 1).c_str());
    return result;
}


class VmbManager{
    public:
        VmbManager(std::map<int, std::string> cam_map, fulfil::utils::Logger* logger);


    private:
        void RunManager();
        void SendResponse(std::string cmd_id, std::string response);
        std::map<int, std::shared_ptr<VmbCamera>> cameras_;
        fulfil::utils::Logger* log_;        
        VmbCPP::VmbSystem& vmb_system = VmbCPP::VmbSystem::GetInstance();
        bool SaveImages(cv::Mat bag_image, std::string image_path);
        void HandleRequest(std::shared_ptr<DepthCameras::DcRequest> request);
        std::shared_ptr<GrpcService> service_;

};

// TODO define this in one location, it is also defined in Fulfil.Dispense lib
enum class DispenseCommand
{
    /// No operation (not sure why this exists).
    nop = 0,
    /// Command sent to acquire a drop target.
    drop_target = 1,
    /// Command sent after a drop occurs.
    post_LFR = 2,
    /// Command sent to start recording tray video in separate thread
    start_tray_video = 3,
    /// Command sent to stop recording tray video in separate thread
    stop_tray_video = 4,
    /// Command sent to determine height of tray.
    tray_validation = 5,
    /// Command sent to find the distance of items from edge of tray.
    item_edge_distance = 6,
    /// Command sent to indicate LFB is arriving at bay, get state from Mongo
    get_state = 7,
    /// Command sent prior to a drop occurs, for LFB camera processing
    pre_LFR = 8,
    /// Command sent to stop a certain command.
    update_state = 9,
    /// Command sent to start recording LFB video in separate thread
    start_lfb_video = 11,
    /// Command sent to stop recording LFB video in separate thread
    stop_lfb_video = 12,
    /// Command sent to home LFB camera rail motor
    home_motor = 20,
    /// Command sent to control rail motor for positioning the LFB camera (absolute position relative to home)
    position_motor = 21,
    /// Command sent to view the floor between dispenses using LFB camera processing
    floor_view = 22,
    /// Command sent to view the tray using tray camera processing
    tray_view = 23,
    /// FC requests the current bag state
    request_bag_state = 30,
    /// FC is sending the new bag state
    send_bag_state    = 31,
    /// Finds safe zone for FC to target extends in side dispense
    side_dispense_target = 32,
    /// Images bag post side dispense
    post_side_dispense = 33,
    /// May get deprecated soon. Pre side dispense image
    pre_side_dispense = 34,
    /// Bag release image at repack cabinet
    bag_release = 35

};