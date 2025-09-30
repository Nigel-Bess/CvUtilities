//
// Created by nkaffine on 12/10/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_DISPENSE_MANAGER_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_DISPENSE_MANAGER_H_
#include <memory>
#include <Fulfil.CPPUtils/commands/dc_api_error_codes.h>
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include <Fulfil.CPPUtils/processing_queue.h>
#include <Fulfil.CPPUtils/networking/socket_network_manager.h>
#include <Fulfil.CPPUtils/networking/socket_network_manager_delegate.h>
#include <Fulfil.DepthCam/core/session.h>
#include <Fulfil.DepthCam/data/upload_generator.h>
#include <Fulfil.Dispense/bays/bay_runner.h>
#include <Fulfil.Dispense/commands/dispense_request.h>
#include <Fulfil.Dispense/commands/dispense_request_delegate.h>
#include <Fulfil.Dispense/commands/dispense_response.h>
#include <Fulfil.Dispense/commands/item_edge_distance/item_edge_distance_response.h>
#include <Fulfil.Dispense/commands/post_drop/post_LFR_response.h>
#include <Fulfil.Dispense/drop/drop_manager.h>
#include <Fulfil.Dispense/tray/tray_algorithm.h>
#include <Fulfil.Dispense/tray/tray_manager.h>
#include <Fulfil.Dispense/commands/parsing/tray_parser.h>
#include <Fulfil.Dispense/visualization/live_viewer.h>

namespace fulfil::dispense {
/**
 * The purpose of this class is to manage the dispense functionality
 * for a given bay.
 */
    class DispenseManager : public std::enable_shared_from_this<DispenseManager>,
                            public fulfil::dispense::bays::BayRunner,
                            public fulfil::utils::networking::SocketNetworkManagerDelegate<std::shared_ptr<fulfil::dispense::commands::DispenseRequest>>,
                            public fulfil::utils::processingqueue::ProcessingQueueDelegate<std::shared_ptr<fulfil::dispense::commands::DispenseRequest>, std::shared_ptr<fulfil::dispense::commands::DispenseResponse>>,
                            public fulfil::dispense::commands::DispenseRequestDelegate
    {
        private:
            /**
             * camera to tray distance at induction station. Used for cropping the image for RTA.
             * 
            */
            int induction_cam_distance_from_tray = 1355; // in mm

            /**
            * The id of the bay this dispense manager is managing.
            */
            int bay;

            /**
             * The object that will manage all of the socket communication and pass along things
             * that are read from the socket.
             *
             */
            std::shared_ptr<fulfil::utils::networking::SocketNetworkManager<std::shared_ptr<fulfil::dispense::commands::DispenseRequest>>> network_manager;
            /**
             * Encapsulates functionality construct tray relevant objects and run
             * tray algorithms
             */
            std::shared_ptr<fulfil::dispense::tray::TrayManager> tray_manager;
            /**
             * The object that determines which requests get processed
             * when. Objects get added to the queue asynchronously but
             * are executed sequentially.
             */
            std::shared_ptr<fulfil::utils::processingqueue::ProcessingQueue<std::shared_ptr<fulfil::dispense::commands::DispenseRequest>, std::shared_ptr<fulfil::dispense::commands::DispenseResponse>>> processing_queue;

            /**
             * The name of the bay this dispense manager is managing.
             */
            std::string machine_name;

            /**
             * The mongo id of the VLS
             */
            std::string machine_id;

            /**
             * The name of the config section for the tray_dimensions in tray_config.ini we want to use, corresponding to 2.0, 2.1 or 3.1
             */
            std::string tray_dimension_type;


            /**
             * Sessions to keep track of session for LFB camera and tray camera separately on a per-bay basis
             */
            std::shared_ptr<fulfil::depthcam::Session> LFB_session;
            std::shared_ptr<fulfil::depthcam::Session> tray_session;


            std::shared_ptr<fulfil::dispense::visualization::LiveViewer> live_viewer;
            std::shared_ptr<fulfil::depthcam::data::UploadGenerator> lfb_upload_generator;
            std::shared_ptr<fulfil::depthcam::data::UploadGenerator> tray_upload_generator;

            std::string store_id;
            std::string cloud_media_bucket;



            /**
             * dispense manager config information (mostly relating to visualization -- see main_config.ini for required info)
             * Should contain should_visualize, data_gen_image_base_dir, vid_gen_base_buffer_dir,
             */
            std::shared_ptr<INIReader>  dispense_reader;
            std::shared_ptr<INIReader>  tray_config_reader;

            /**
             *  Mongo id for the LFB bag currently being inspected at the bay
             */
            std::string bag_id;

            /**
            *  keeps track of whether the bot has already been commanded to rotate 180 degrees for the current dispense
            * (i.e. the Drop Target result Rotate_LFB was True for the first drop target request for the current dispense)
            * This is important because we will only command the bot to rotate a maximum of one time per dispense
            */
            bool bot_already_rotated_for_current_dispense = false;


            /**
             * Builds a fresh copy of base_dir from configs and appends a date to the end.
             * Will throw an exception if it fails to find the base directory.
             * @return shared pointer to a date modified basedir i.e
             * /home/fulfil/Videos/saved_images_2021_04_29
             */
            std::shared_ptr<std::string> create_datagenerator_basedir();
            std::shared_ptr<fulfil::utils::networking::SocketManager> socket_manager;




        public:
            /**
             * DispenseManager constructor
             * @param bay the id of the bay that this manager will manage.
             * @param sensor session that will be used to complete requests
             * for this manager.
             * @param should_visualize true if the manager should use visualization
             * code such as displaying the drop zone (should be false if there is
             * no display to prevent crashes)
             */
          DispenseManager(
              int bay, std::shared_ptr<fulfil::depthcam::Session> LFB_session,
              std::shared_ptr<fulfil::depthcam::Session> tray_session,
              std::shared_ptr<INIReader> dispense_man_reader,
              std::shared_ptr<INIReader> tray_config_reader,
              std::shared_ptr<fulfil::dispense::tray::TrayManager>
                  tray_manager);

            /**
             * The object that handles all of the functionality for processing
             * drop requests.
             */
            std::shared_ptr<fulfil::dispense::drop::DropManager> drop_manager;
            
            ///Mark: Bay Runner Functions
            /**
             * For each object owned by this object where this object should
             * be the delegate, assigns a weak pointer of itself to the delegate
             * of those objects. Needs to have a function like this to make sure
             * it only attaches itself as a delegate after a shared pointer to
             * the object exists to ensure valid weak pointers are created.
             */
            void bind_delegates() override;
            /**
             * Initiates the process of listening for requests and processing
             * requests.
             */
            void start() override;
            /**
             * Returns a shared pointer to this object as a bay runner.
             * @return shared pointer to this object as a bay runner.
             */
            std::shared_ptr<fulfil::dispense::bays::BayRunner> get_shared_ptr() override ;

            ///Mark: Socket Network Manager Delegate Functions
            void did_receive_request(std::shared_ptr<fulfil::dispense::commands::DispenseRequest> request) override;

            void did_receive_invalid_request(std::shared_ptr<std::string> request_id) override;

            void did_receive_invalid_request() override;

            /// Mark: Processing Queue Delegate Functions
            void send_response(std::shared_ptr<fulfil::dispense::commands::DispenseResponse> response) override;

            std::shared_ptr<fulfil::dispense::commands::DispenseResponse> process_request(std::shared_ptr<fulfil::dispense::commands::DispenseRequest> request) override;

            /// Mark: Dispense Command Delegate Functions
            std::shared_ptr<fulfil::dispense::drop::DropResult> handle_drop_target(std::shared_ptr<fulfil::dispense::commands::DropTargetDetails> details,
                    std::shared_ptr<nlohmann::json> request_json) override;

            std::shared_ptr<fulfil::dispense::commands::ItemEdgeDistanceResponse> handle_item_edge_distance(std::shared_ptr<std::string> command_id,
                                                                                std::shared_ptr<nlohmann::json> request_json) override;

            /// Mark: Post-Dispense Command Delegate Function
            std::shared_ptr<fulfil::dispense::commands::PostLFRResponse> handle_post_LFR(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<std::string> request_id, std::shared_ptr<nlohmann::json> request_json) override;

            std::shared_ptr<fulfil::dispense::commands::FloorViewResponse> handle_floor_view(std::shared_ptr<std::string> PrimaryKeyID,
                                                                                             std::shared_ptr<std::string> command_id,
                                                                                             std::shared_ptr<nlohmann::json> request_json) override;

            std::shared_ptr<fulfil::dispense::commands::TrayViewResponse> handle_tray_view(std::shared_ptr<std::string> PrimaryKeyID,
                                                                                            std::shared_ptr<std::string> command_id,
                                                                                            std::shared_ptr<nlohmann::json> request_json) override;

            /// Mark: Start LFB Video Command Delegate Function
            void handle_start_lfb_video(std::shared_ptr<std::string> PrimaryKeyID) override;

            /// Mark: Stop LFB Video Command Delegate Function
            void handle_stop_lfb_video() override;

            /// Mark: Start Tray Video Command Delegate Function
            void handle_start_tray_video(std::shared_ptr<std::string> PrimaryKeyID) override;

            /// Mark: Stop Tray Video Command Delegate Function
            // delay is an integer value corresponding to ms units
            void handle_stop_tray_video(int delay) override;

            void handle_stop_request(std::shared_ptr<std::string> command_id) override;

            std::shared_ptr<fulfil::dispense::commands::TrayValidationResponse>
            handle_tray_validation(std::shared_ptr<std::string> command_id, std::shared_ptr<nlohmann::json> request_json) override;

            std::string  handle_get_state(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json) override;

            int handle_update_state(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json) override;

            std::shared_ptr<fulfil::dispense::commands::PreSideDispenseResponse> handle_pre_side_dispense(std::shared_ptr<std::string> request_id, std::shared_ptr<nlohmann::json> request_json) override;
            std::shared_ptr<fulfil::dispense::commands::SideDispenseTargetResponse> handle_side_dispense_target(std::shared_ptr<std::string> request_id, std::shared_ptr<nlohmann::json> request_json) override;
            std::shared_ptr<fulfil::dispense::commands::PostSideDispenseResponse> handle_post_side_dispense(std::shared_ptr<std::string> request_id, std::shared_ptr<nlohmann::json> request_json) override;

            std::shared_ptr<fulfil::dispense::commands::CalibrateTrayDepthCameraResponse> handle_tray_camera_calibration(
                std::shared_ptr<std::string> request_id,
                std::shared_ptr<nlohmann::json> request_json
            ) override;

            
            std::shared_ptr<fulfil::dispense::commands::PrePickupClipActuatorResponse> handle_pre_pickup_clip_actuator(std::shared_ptr<std::string> request_id, std::shared_ptr<nlohmann::json> request_json) override;
            // Currently always returns true to meet FCs expectations after motor control outsourced to fw
            bool check_motor_in_position() override;

            void handle_request_in_thread(std::shared_ptr<std::string> payload, std::shared_ptr<std::string> command_id);
            // TODO: delete, unused
            void handle_request(std::shared_ptr<std::string> payload, std::shared_ptr<std::string> command_id) override;

            std::shared_ptr<fulfil::dispense::commands::CodeResponse> handle_pre_LFR(
                std::shared_ptr<std::string> PrimaryKeyID,
                std::shared_ptr<std::string> command_id,
                std::shared_ptr<nlohmann::json> request_json
            ) override;

            int get_induction_cam_distance_from_tray();

    };
} // namespace fulfil::dispense

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_DISPENSE_MANAGER_H_
