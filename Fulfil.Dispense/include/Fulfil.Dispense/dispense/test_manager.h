#pragma once

#include <memory>
#include <Fulfil.Dispense/bays/bay_runner.h>
#include <Fulfil.DepthCam/core/session.h>
#include <Fulfil.CPPUtils/networking/socket_network_manager_delegate.h>
#include <Fulfil.Dispense/commands/post_drop/post_LFR_response.h>
#include <Fulfil.Dispense/commands/dispense_response.h>
#include <Fulfil.Dispense/commands/item_edge_distance/item_edge_distance_response.h>
#include <Fulfil.CPPUtils/processing_queue.h>
#include <Fulfil.Dispense/commands/dispense_request_delegate.h>
#include <Fulfil.Dispense/commands/dispense_request.h>
#include <Fulfil.CPPUtils/networking/socket_network_manager.h>
#include <Fulfil.Dispense/drop/drop_manager.h>
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include <Fulfil.Dispense/visualization/live_viewer.h>
#include <FulfilMongoCpp/mongo_connection.h>
#include <Fulfil.Dispense/tray/tray_parser.h>
#include <Fulfil.Dispense/tray/tray_algorithm.h>
#include <Fulfil.Dispense/tray/tray_result.h>
#include <Fulfil.Dispense/tray/tray_manager.h>
#include <Fulfil.Dispense/motion/trinamic.h>
#include <Fulfil.DepthCam/data/upload_generator.h>
#include <Fulfil.DepthCam/data/bigquery_upload.h>

namespace fulfil::dispense {
        class TestManager : public std::enable_shared_from_this<TestManager>,
                                public fulfil::utils::networking::SocketNetworkManagerDelegate<std::shared_ptr<fulfil::dispense::commands::DispenseRequest>>,
                                public fulfil::utils::processingqueue::ProcessingQueueDelegate<std::shared_ptr<fulfil::dispense::commands::DispenseRequest>, std::shared_ptr<fulfil::dispense::commands::DispenseResponse>>,
                                public fulfil::dispense::commands::DispenseRequestDelegate
                                
        {
        private:
            std::shared_ptr<fulfil::utils::networking::SocketNetworkManager<std::shared_ptr<fulfil::dispense::commands::DispenseRequest>>> network_manager;
            std::shared_ptr<fulfil::utils::processingqueue::ProcessingQueue<std::shared_ptr<fulfil::dispense::commands::DispenseRequest>, std::shared_ptr<fulfil::dispense::commands::DispenseResponse>>> processing_queue;
            std::string bag_id;

        public:
          TestManager();
          void run_stream(std::string serial);
            void bind_delegates();
            void start();
            void did_receive_request(std::shared_ptr<fulfil::dispense::commands::DispenseRequest> request);

            void did_receive_invalid_request(std::shared_ptr<std::string> request_id) override;

            void did_receive_invalid_request() override;

            /// Mark: Processing Queue Delegate Functions
            void send_response(std::shared_ptr<fulfil::dispense::commands::DispenseResponse> response) override;

            std::shared_ptr<fulfil::dispense::commands::DispenseResponse> process_request(std::shared_ptr<fulfil::dispense::commands::DispenseRequest> request) override;

            std::string  handle_get_state(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json) ;

            int handle_update_state(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json);

            std::shared_ptr<fulfil::mongo::MongoBagState> mongo_bag_state;

            /// Mark: Dispense Command Delegate Functions
            inline std::shared_ptr<fulfil::dispense::drop::DropResult> handle_drop_target(std::shared_ptr<fulfil::dispense::commands::DropTargetDetails> details,
                    std::shared_ptr<nlohmann::json> request_json) override {return nullptr;}

           inline  std::shared_ptr<fulfil::dispense::commands::ItemEdgeDistanceResponse> handle_item_edge_distance(std::shared_ptr<std::string> command_id,
                                                                                std::shared_ptr<nlohmann::json> request_json) override {return nullptr;}

            /// Mark: Post-Dispense Command Delegate Function
            inline std::shared_ptr<fulfil::dispense::commands::PostLFRResponse> 
            handle_post_LFR(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<std::string> request_id, std::shared_ptr<nlohmann::json> request_json) override {return nullptr;}

            /// Mark: Start LFB Video Command Delegate Function
            inline void handle_start_lfb_video(std::shared_ptr<std::string> PrimaryKeyID) override {}

            /// Mark: Stop LFB Video Command Delegate Function
            inline void handle_stop_lfb_video() override {}

            /// Mark: Start Tray Video Command Delegate Function
            inline void handle_start_tray_video(std::shared_ptr<std::string> PrimaryKeyID) override {}

            /// Mark: Stop Tray Video Command Delegate Function
            // delay is an integer value corresponding to ms units
           inline  void handle_stop_tray_video(int delay) override {}

            inline void handle_stop_request(std::shared_ptr<std::string> command_id) override {}

            inline std::shared_ptr<fulfil::dispense::commands::TrayValidationResponse>
            handle_tray_validation(std::shared_ptr<std::string> command_id, std::shared_ptr<nlohmann::json> request_json) override { return nullptr;}

            inline int handle_home_motor(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json) override {return 1;}

           inline int handle_position_motor(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json) override {return 1;}

            /**
            *  Returns true if rail motor is stationary after a position or home motion
            */
            inline bool check_motor_in_position() override {return false;}

            void handle_request(std::shared_ptr<std::string> payload, std::shared_ptr<std::string> command_id) override;



           inline  int handle_pre_LFR(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json) override {return 1;}

        };
    } // namespace fulfil

