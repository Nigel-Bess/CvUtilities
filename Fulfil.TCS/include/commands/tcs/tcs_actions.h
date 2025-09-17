#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TCS_ACTIONS_H
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TCS_ACTIONS_H

#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/comm/GrpcService.h>
#include <Fulfil.CPPUtils/orbbec/orbbec_manager.h>
#include "tcs_perception.h"
#include "tcs_error_codes.h"
#include "tcs_response.h"
#include <Fulfil.CPPUtils/commands/dispense_command.h>

namespace fulfil::dispense::commands::tcs {

    /**
     * Pure JSON request/response handling with images/depths already read by other camera layers or otherwise
     * hardcoded in tests.
     */
    class TCSActions {
        public:
            TCSActions(TCSPerception* tcs_perception);
            std::shared_ptr<DepthCameras::DcResponse> handle_pre_pick_clip_actuator_request(cv::Mat color_bag_clips_img, std::string &pkid, std::string &cmd_id);

        private:
            TCSPerception* tcs_perception;
            std::shared_ptr<DcResponse> to_response(std::string &cmd_id, std::string &json_response);
    };
}

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TCS_ACTIONS_H