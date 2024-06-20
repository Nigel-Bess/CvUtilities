//
// Created by steve on 12/31/20.
//

#ifndef FULFIL_COMPUTERVISION_POST_SIDE_DISPENSE_REQUEST_H
#define FULFIL_COMPUTERVISION_POST_SIDE_DISPENSE_REQUEST_H

#include <Fulfil.Dispense/commands/dispense_request.h>
#include <Fulfil.Dispense/commands/dispense_response.h>

namespace fulfil::dispense::commands {

class PreSideDispenseRequest : public fulfil::dispense::commands::DispenseRequest
{
 public:

  std::shared_ptr<nlohmann::json> request_json;

  explicit PreSideDispenseRequest(std::shared_ptr<std::string> command_id, std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json);

  std::shared_ptr<fulfil::dispense::commands::DispenseResponse> execute() override;
};
} // namespace fulfil::dispense::commands 


#endif //FULFIL_COMPUTERVISION_POST_SIDE_DISPENSE_REQUEST_H

