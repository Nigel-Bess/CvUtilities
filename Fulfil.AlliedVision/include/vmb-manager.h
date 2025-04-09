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
        void HandleRequest(std::shared_ptr<DepthCameras::DcRequest> request, int remainingRetries);
        std::shared_ptr<GrpcService> service_;

};
