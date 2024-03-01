//
// Created by nkaffine on 12/10/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include "Fulfil.Dispense/dispense/realsense_bay_parser.h"

using fulfil::utils::Logger;
using fulfil::dispense::RealsenseBayParser;

RealsenseBayParser::RealsenseBayParser(std::shared_ptr<INIReader> reader) : device_to_dispense_config{reader},
    bay_ids(device_to_dispense_config->GetSpaceSepStrVector(device_to_dispense_config->get_default_section(), "dispenses"))
{
    if (bay_ids.empty()) { bay_ids.push_back("0") ; }
}

std::vector<std::string> RealsenseBayParser::get_camera_ids() {
    //auto expected_bays = device_to_dispense_config->GetIntegerVector(device_to_dispense_config->get_default_section(), "dispenses", {0});
    std::vector<std::string> camera_names = {"LFB_cam", "tray_cam"};
    std::vector<std::string> serials;
    for(auto bay : bay_ids) {
        auto dispense_name = "dispense_" + bay;
        std::for_each(camera_names.begin(), camera_names.end(), [&reader=device_to_dispense_config, &dispense_name, &serials](auto cam_name) {
            auto serial = reader->Get(dispense_name, cam_name, "err");
            if (serial != "err") {
                serials.emplace_back(serial);
                fulfil::utils::Logger::Instance()->Info("{}::{} {} is expected to be {}",
                  reader->Get(dispense_name, "name", dispense_name), reader->Get(dispense_name, "_id", dispense_name), cam_name, serial);
            }

        });
    }
    return serials;
}


std::vector<std::shared_ptr<fulfil::depthcam::Session>> RealsenseBayParser::get_bay(std::shared_ptr<std::vector<std::shared_ptr<fulfil::depthcam::Session>>> sessions,
  const std::string &config_id)
{
    auto dispense_name = "dispense_" + config_id;
    auto grab_cam = [&sessions, &dispense_name, &conf=this->device_to_dispense_config]
      (auto cam_name) {
        auto serial_search = [serial=conf->Get(dispense_name, cam_name, "err")]
        (std::shared_ptr<fulfil::depthcam::Session> s ) { return *(s->get_serial_number()) == serial; };
        if (auto serial_match = std::find_if(sessions->begin(), sessions->end(), serial_search) ; serial_match!=sessions->end()){
            return *serial_match;
        }
        fulfil::utils::Logger::Instance()->Warn("Could not find sensor for camera {}::{} with expected id {}", dispense_name, cam_name, conf->Get(dispense_name, cam_name, "err"));
        return std::shared_ptr<fulfil::depthcam::Session>(nullptr);
    };

  //fulfil::utils::Logger::Instance()->Warn("Could not find sensor to match camera {}: {}", config_num, *this->bay_serials->at(config_num));
  return { grab_cam("lfb_cam"), grab_cam("tray_cam")};
}
std::vector<std::string> fulfil::dispense::RealsenseBayParser::get_bay_ids() { return bay_ids; }

fulfil::dispense::DispenseBayData::DispenseBayData(std::vector<std::shared_ptr<fulfil::depthcam::Session>> sessions, std::string bay_id, bool both_required) :
  bay_id(bay_id), both_required(both_required)
    {
        //TODO get rid of this wtf
        if (sessions.size() != 2) { throw std::runtime_error("Must send vector size 2 for bay sessions!") ;}
        lfb_session=sessions.front();
        tray_session=sessions.back();
    }

bool fulfil::dispense::DispenseBayData::can_init(){
  return (both_required && lfb_session && tray_session) || (!both_required && (lfb_session || tray_session));
}
int fulfil::dispense::DispenseBayData::num_valid_session() { return (lfb_session ? 1 : 0) + (tray_session ? 1 : 0); }
