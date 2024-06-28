//
// Created by amber on 10/12/20.
//

#include <Fulfil.Dispense/commands/item_edge_distance/item_edge_distance_request.h>
#include <Fulfil.Dispense/commands/dispense_request_delegate.h>
#include <Fulfil.Dispense/commands/item_edge_distance/item_edge_distance_response.h>


using fulfil::dispense::commands::DispenseRequestDelegate;
using fulfil::dispense::commands::ItemEdgeDistanceRequest;

ItemEdgeDistanceRequest::ItemEdgeDistanceRequest(std::shared_ptr<std::string> command_id,
                                                 std::shared_ptr<std::string> PrimaryKeyID,
                                                 std::shared_ptr<nlohmann::json> request_json)
{
  this->request_id = command_id;
  this->PrimaryKeyID = PrimaryKeyID;
  this->request_json = request_json;
}

std::shared_ptr<fulfil::dispense::commands::DispenseResponse> ItemEdgeDistanceRequest::execute()
{
  if(!this->delegate.expired())
  {
    std::shared_ptr<DispenseRequestDelegate> tmp_delegate = this->delegate.lock();
    std::shared_ptr<fulfil::dispense::commands::ItemEdgeDistanceResponse> item_distance_response =
            tmp_delegate->handle_item_edge_distance(request_id, request_json); // calling method in dispense_manager
    return item_distance_response;
  }
  else
  {
    std::cout << "ItemEdgeDistance Command Delegate Expired" << std::endl;
    return std::make_shared<ItemEdgeDistanceResponse>(this->request_id, 9); //Todo: change the error code used here if needed
  }
}
