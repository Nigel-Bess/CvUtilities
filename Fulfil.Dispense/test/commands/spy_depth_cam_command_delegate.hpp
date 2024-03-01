#include "../../commands/depth_cam_command_delegate.hpp"
#include "../../drop_search/drop_result.hpp"

#ifndef SPY_DEPTH_CAM_COMMAND_DELEGATE_H_
#define SPY_DEPTH_CAM_COMMAND_DELEGATE_H_

class SpyDepthCamCommandDelegate : public DepthCamCommandDelegate, public enable_shared_from_this<SpyDepthCamCommandDelegate>
{
public:
    std::shared_ptr<PreDispenseDetails> pre_dispense_details;
    std::shared_ptr<char*> stop_request_id;

    SpyDepthCamCommandDelegate()
    {
        this->stop_request_id = nullptr;
        this->pre_dispense_details = nullptr;
    }

    std::shared_ptr<DropResult> handle_pre_dispense(std::shared_ptr<PreDispenseDetails> details)
    {
        this->pre_dispense_details = details;
    }

    void handle_stop_request(std::shared_ptr<char*> command_id)
    {
        this->stop_request_id = command_id;
    }
};

#endif