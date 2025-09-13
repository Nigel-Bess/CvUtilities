#pragma once

#include <json.hpp>
#include <Fulfil.CPPUtils/orbbec/orbbec_camera.h>

using fulfil::utils::orbbec::OrbbecCamera;

class OrbbecManager{
    public:
        OrbbecManager(fulfil::utils::Logger* logger) : logger(logger) {};
        void start_manager();
        std::map<std::string, std::shared_ptr<OrbbecCamera>>* get_cameras_by_name();

    private:
        std::map<std::string, std::shared_ptr<OrbbecCamera>> name_to_cam;
        fulfil::utils::Logger* logger;

        std::shared_ptr<OrbbecCamera> get_camera(std::string name);
        void run_manager();
};
