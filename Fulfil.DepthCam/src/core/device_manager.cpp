/**
 * This file contains the implementation of the device manager that
 * interacts with the realsense sdk to create sessions that can be
 * used with other code in this library.
 */
#include"Fulfil.DepthCam/core/device_manager.h"
#include "../presets/device_presets.h"
#include <iostream>
#include <librealsense2/rs_advanced_mode.hpp>
#include "Fulfil.DepthCam/json.hpp"
// wtf need to fix this so there is only one header
#include <Fulfil.CPPUtils/logging.h>
#include <experimental/filesystem>
#include <fstream>


namespace std_filesystem = std::experimental::filesystem;
using fulfil::utils::Logger;
using fulfil::depthcam::DeviceManager;
using fulfil::depthcam::DepthSession;
using fulfil::depthcam::Session;



DeviceManager::DeviceManager() :
                                 managed_devices(get_managed_device_list()),
                                 in_frozen_env(false)
{
    this->reset_context_list();
}

DeviceManager::DeviceManager(std::vector<std::string> expected_devices, bool frozen) :
                                                                          managed_devices(get_managed_device_list(expected_devices)),
                                                                          in_frozen_env(frozen)
{
    this->reset_context_list();
}

std::string unwrap_rs_serial(const rs2::device& _dev) {
    return _dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
}

void DeviceManager::reset_context_list() {
    auto num_devices = this->context.query_devices().size();
    auto num_managed_devices = managed_devices.size();
    fulfil::utils::Logger::Instance()->Info("Managing {} realsense depth cameras out of {} devices connected.",
      num_managed_devices, num_devices);

    auto hub = rs2::device_hub{this->context} ;
    context.set_devices_changed_callback( [num_devices, num_managed_devices](rs2::event_information info)
      { std::string action_msg = info.get_new_devices().size() ? "New device(s) added! First is " + unwrap_rs_serial(info.get_new_devices().front()) : "Device removed!";
          fulfil::utils::Logger::Instance()->Info("DEVICE EVENT OCCURRED: Number new devices is {}. {} There were {} managed and {} total at api start.",
          info.get_new_devices().size(), action_msg, num_managed_devices, num_devices); });
    auto lockstate = [] (rs2::device& _dev ) { return _dev.get_info(RS2_CAMERA_INFO_CAMERA_LOCKED) ? "ON" : "OFF" ; };
    auto connected = [&hub] (rs2::device& _dev ) { return hub.is_connected(_dev) ? "Connected" : "DISCONNECTED" ; };
    for(rs2::device device : managed_devices)
    {
        fulfil::utils::Logger::Instance()->Info("** Init hw status ** Type: {}, Device: {}, Lock: {}, Status: {}", device.get_info(RS2_CAMERA_INFO_NAME), unwrap_rs_serial(device), lockstate(device), connected(device));
        device.hardware_reset();
        hub.wait_for_device(); //Not sure that this actually blocks on D457s
        sleep(2);
        fulfil::utils::Logger::Instance()->Info("** After hw reset ** Type: {}, Device: {}, Lock: {}, Status: {}",
          device.get_info(RS2_CAMERA_INFO_NAME), unwrap_rs_serial(device), lockstate(device), connected(device));
    }
    if (!managed_devices.empty()) { sleep(2) ;};
    fulfil::utils::Logger::Instance()->Info("Reset {} managed devices for production...", managed_devices.size());
}

std::shared_ptr<Session> DeviceManager::session_by_serial_number(const std::string &serial_number)
{   //Checks to see if the specified device is connected.
    for(rs2::device const device : this->context.query_devices()) {
        if(std::string_view{device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER)} == serial_number){
            fulfil::utils::Logger::Instance()->Debug("Found specified device {}", serial_number);
            return std::make_unique<DepthSession>(std::make_unique<DepthSensor>(serial_number));
        }
        fulfil::utils::Logger::Instance()->Debug("Found {} which does not match {}",
          device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER),  serial_number);

    }
    fulfil::utils::Logger::Instance()->Error("Failed to find session {} in context", serial_number);
    throw std::invalid_argument("serial number does not correspond to any connected devices");
}




std::vector<rs2::device> DeviceManager::get_managed_device_list() const {
    return this->context.query_devices();
}
// May also make init list if want to filter cams out
std::vector<rs2::device> DeviceManager::get_managed_device_list(std::vector<std::string> greenlit_devices) const {
    if (greenlit_devices.empty()) return get_managed_device_list();
    rs2::device_list all_devices = this->context.query_devices();
    std::vector<rs2::device> managed_detected_devices{};
    auto is_greenlit = [&greenlit_devices](rs2::device dev) {
        fulfil::utils::Logger::Instance()->Info("Found connected realsense device {}...", (unwrap_rs_serial(dev)));
        return std::any_of(greenlit_devices.cbegin(), greenlit_devices.cend(),
          [&dev](auto serial) { return serial == unwrap_rs_serial(dev) ; });
    };
    std::copy_if(all_devices.begin(), all_devices.end(), std::back_inserter(managed_detected_devices),
      is_greenlit);
    return managed_detected_devices;
}


std::vector<std::string> DeviceManager::get_device_serials() const
{
    std::vector<std::string> serials{};
    std::transform(
      this->managed_devices.begin(), this->managed_devices.end(), std::back_inserter(serials),
      [](rs2::device device) { return unwrap_rs_serial(device);
      });
    return serials;
}




nlohmann::json parse_request_file_to_json(std_filesystem::path json_file_path) {
  // Parse request Json
  std::ifstream json_file(json_file_path);
  nlohmann::json json_obj_data;
  json_file >> json_obj_data;
  return json_obj_data;
}


std::shared_ptr<std::vector<std::shared_ptr<Session>>> DeviceManager::get_connected_sessions()
{
    std::shared_ptr<std::vector<std::shared_ptr<Session>>> result =
            std::make_shared<std::vector<std::shared_ptr<Session>>>();

    for(rs2::device const device : this->managed_devices)
    {
        fulfil::utils::Logger::Instance()->Trace("Getting device serial number from list...");
        const char* device_serial_number = device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
        std::string name = device.get_info(RS2_CAMERA_INFO_NAME);
        fulfil::utils::Logger::Instance()->Trace("Current device {} name is: {}", unwrap_rs_serial(device), name);
        fulfil::utils::Logger::Instance()->Trace("Found device serial number from list...");
        auto D415_presets = in_frozen_env ? *DevicePresets::D415_frozen() : *DevicePresets::D415_high_accuracy();

        if(device.is<rs400::advanced_mode>())
        {
            auto advanced_mode_dev = device.as<rs400::advanced_mode>();
            if (name == "Intel RealSense D415"){
                advanced_mode_dev.load_json(*DevicePresets::D415_high_accuracy());
                fulfil::utils::Logger::Instance()->Info("Loaded advanced mode for D415 device {}...", device_serial_number);
            }
            else if (name == "Intel RealSense D455"){
                advanced_mode_dev.load_json(*DevicePresets::D455_adjusted());
                fulfil::utils::Logger::Instance()->Info("Loaded advanced mode for D455 device {}...", device_serial_number);
            }
            else if (name == "Intel RealSense D457"){
              std_filesystem::path preset_base_dir = std_filesystem::path(fulfil::utils::Logger::default_logging_dir).parent_path();
              preset_base_dir /= "libs/Fulfil.DepthCam/src/presets/d457_adjustments.json";
              auto presets = parse_request_file_to_json(preset_base_dir);
              fulfil::utils::Logger::Instance()->Info("D457 advance mode settings for {} at {}...", device_serial_number, preset_base_dir.string());
              advanced_mode_dev.load_json(presets.dump());
              fulfil::utils::Logger::Instance()->Info("Loaded advanced mode for D457 device {}...", device_serial_number);
            } else{
                fulfil::utils::Logger::Instance()->Error("Did not load presets for advanced mode. Device neither D415, D455 "
                                                        "nor D457, but {}", name);
            }
        } else {
          fulfil::utils::Logger::Instance()->Info("Device {} does not support advanced mode", device_serial_number);
        }
        result->emplace_back(std::make_shared<DepthSession>(std::make_unique<DepthSensor>(device_serial_number)));
        fulfil::utils::Logger::Instance()->Trace("Session added to result.");
    }
    return result;
}



std::shared_ptr<Session> DeviceManager::get_first_connected_session(int maximum_tries)
{
    int current_try = 0;
    while(current_try < maximum_tries || maximum_tries == -1)
    {
        try
        {
            auto sessions = this->get_connected_sessions();
            if(sessions->empty())
            {
                throw std::invalid_argument("no sensor detected");
            }
            //Just grabs the first session
            return sessions->at(0);
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
            current_try += 1;
        }
    }
    throw std::runtime_error("no sensor was detected within maximum tries");
}
