#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TCS_CONTROLLER_H
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TCS_CONTROLLER_H

#include <Fulfil.CPPUtils/commands/dispense_command.h>
#include "tcs_actions.h"

namespace fulfil::dispense::commands::tcs {

    struct GrpcPort {
        GrpcPort(std::string name, GrpcService *service) : name(name), service(service) {};
        std::string name;
        GrpcService *service;
    };

    class TCSController {
        public:
            /**
             * Initialize a new TCSContoller TOOD: Also inject Vimba manager
             */
            TCSController(TCSPerception* tcs_perception, OrbbecManager* orbbec_manager);
            /**
             * Listen over Grpc ports configured by env vars
             */
            void start_listening();
            /**
             * Stop listening over Grpc and gracefully close all grpc ports opened from last listen call, does NOT cleanup
             * resources provided to the constructor! Parent logic has to do that.
             */
            void stop_listening();

        private:
            volatile bool is_listening;
            TCSPerception* tcs_perception;
            TCSActions* actions;
            OrbbecManager* orbbec_manager;
            GrpcPort* open_port(std::string cam_name);
            void update_port_status(GrpcPort *port, DepthCameras::DcCameraStatusCodes code);
            void listen_loop();
            void handle_request(GrpcPort *port, std::shared_ptr<DepthCameras::DcRequest> request, int remainingRetries);
            std::shared_ptr<DcResponse> to_response(std::string &cmd_id, std::shared_ptr<nlohmann::json> result_json);
    };
}

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TCS_CONTROLLER_H