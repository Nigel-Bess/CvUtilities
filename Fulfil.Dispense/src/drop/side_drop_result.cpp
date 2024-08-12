#include <memory>
#include <vector>
#include <Fulfil.Dispense/drop/side_drop_result.h>
#include <Fulfil.CPPUtils/logging.h>

using fulfil::dispense::drop::SideDropResult;
using fulfil::utils::Point3D;
using fulfil::utils::Logger;


SideDropResult::SideDropResult(std::shared_ptr<std::string> request_id,
                               std::shared_ptr<std::vector<std::vector<int> > > occupancy_map,
                               int error_code,
                               const std::string &error_description) {
    this->request_id = request_id;
    this->occupancy_map = occupancy_map;
    this->success_code = error_code;
    this->error_description = error_description;
}
