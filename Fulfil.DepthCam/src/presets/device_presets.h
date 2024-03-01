//
// Created by nkaffine on 12/2/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file outlines the static json content to be used as presets
 * in the realsense cameras. As of now this is private because
 * there is no mechanism for dynamically changing the preset.
 *
 * The presets are from the following source: https://dev.intelrealsense.com/docs/d400-series-visual-presets
 * For the high_accuracy presets, the high_accuracy preset params were inserted manually
 */
#ifndef FULFIL_DEPTHCAM__DEVICE_PRESETS_H
#define FULFIL_DEPTHCAM__DEVICE_PRESETS_H
#include <memory>

namespace fulfil
{
    namespace depthcam
    {
        class DevicePresets
        {
        public:
            /**
             * returns the json string with the information for the high accuracy preset for D415.
             * @return pointer to the json string.
             */
            static std::shared_ptr<std::string> D415_high_accuracy();

            /**
             * returns the json string with the information for the adjusted D455 preset for optimal
             * performance.
             * @return pointer to the json string.
             */
            static std::shared_ptr<std::string> D455_adjusted();

            /**
             * returns the json string with the information for the default presets.
             * @return pointer to the json string.
             */
            static std::shared_ptr<std::string> D415_default();
            static std::shared_ptr<std::string> D415_frozen();
            static std::shared_ptr<std::string> D455_default();
        };
    }
}


#endif
