#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TCS_ACTIONS_H
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TCS_ACTIONS_H

#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/comm/GrpcService.h>
#include <Fulfil.OrbbecUtils/orbbec/orbbec_manager.h>
#include "tcs_perception.h"
#include "tcs_error_codes.h"
#include "tcs_response.h"

namespace fulfil::dispense::commands::tcs {

    /**
     * Pure JSON request/response handling with images/depths already read by other camera layers or otherwise
     * hardcoded in tests.
     */
    class TCSActions {
        public:
            TCSActions(TCSPerception* tcs_perception);
            std::shared_ptr<nlohmann::json> handle_pre_pick_clip_actuator_request(cv::Mat color_bag_clips_img, std::shared_ptr<nlohmann::json> request, std::string &cmd_id);

        private:
            TCSPerception* tcs_perception;
    };
}

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TCS_ACTIONS_H