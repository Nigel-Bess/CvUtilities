//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DISPENSE_COMMAND_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DISPENSE_COMMAND_H_

namespace fulfil::dispense::commands
{
    /**
     * Enum that represents the type of commands that
     * are handled by dispense.
     */
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
        /// Pre side dispense image
        pre_side_dispense = 34,
        /// Bag release image at repack cabinet
        bag_release = 35
    };
} // namespace fulfil::dispense::commands

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DISPENSE_COMMAND_H_
