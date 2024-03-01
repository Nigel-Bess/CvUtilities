//
// Created by nkaffine on 12/10/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_REALSENSE_BAY_PARSER_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_REALSENSE_BAY_PARSER_H_
#include <Fulfil.Dispense/bays/bay_parser.h>
#include <Fulfil.DepthCam/core.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/inih/INIReader.h>

namespace fulfil
{
    namespace dispense {
        struct DispenseBayData {
            std::shared_ptr<fulfil::depthcam::Session> lfb_session{ nullptr};
            std::shared_ptr<fulfil::depthcam::Session> tray_session{ nullptr};
            std::string bay_id{"0"};
            bool both_required{true};
            DispenseBayData() = default;
            DispenseBayData(std::vector<std::shared_ptr<fulfil::depthcam::Session>> sessions,  std::string bay_id, bool both_required=false);
            bool can_init();
            int num_valid_session();


        };

        class RealsenseBayParser : public fulfil::dispense::bays::BayParser<std::shared_ptr<fulfil::depthcam::Session>>
        {
        private:
            /**
             * pointer to vector of strings indexed by which bay they belong to. Using
             * the singleton patter because the serial numbers will be stored in a
             * config file and should only be read once and then kept in memory.
             */
          std::shared_ptr<INIReader> device_to_dispense_config;
          std::vector<std::string> bay_ids;
        public:
            /**
             * RealsenseBayParser constructor that initializes the serial numbers.
             */
            RealsenseBayParser(std::shared_ptr<INIReader> reader);
            /**
             * Given the session from the depth cam library, uses the serial number
             * to determine which bay the camera should be assigned to.
             * @param session being assigned
             * @return the identifier of the bay this session is assigned to.
             */
            std::vector<std::string> get_camera_ids();

            std::vector<std::shared_ptr<fulfil::depthcam::Session>> get_bay(std::shared_ptr<std::vector<std::shared_ptr<fulfil::depthcam::Session>>> sessions,
              const std::string &config_id);

            std::vector<std::string> get_bay_ids () override;

        };
    } // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_REALSENSE_BAY_PARSER_H_