#include <memory>
#include <vector>
#include <Fulfil.Dispense/drop/side_drop_result.h>
#include <Fulfil.CPPUtils/logging.h>

using fulfil::dispense::drop::SideDropResult;
using fulfil::utils::Point3D;
using fulfil::utils::Logger;


SideDropResult::SideDropResult(std::shared_ptr<std::string> request_id,
   std::shared_ptr<std::vector<std::shared_ptr<std::vector<int>>>> occupancy_map,
   int error_code,
   const std::string &error_description)
{
  this->request_id = request_id;
  this->occupancy_map = occupancy_map;
  this->success_code = error_code;
  this->error_description = error_description;
}


// DropResult::DropResult(std::shared_ptr<Point3D> drop_center, std::shared_ptr<Point3D> max_depth_point, bool Rotate_LFB, bool LFB_Currently_Rotated,
//                        bool Swing_Collision_Expected,  float target_depth_range, float target_depth_variance, float interference_max_z,
//                        float interference_average_z, float target_region_max_z, std::shared_ptr<std::string> request_id, int success_code, const std::string &error_description)
// {
//     this->depth_result = this->to_rounded_millimeters(drop_center->z);
//     this->dispense_position = this->to_rounded_millimeters(drop_center->y * -1.0); //Notice flip sign so that local bag coordinates transformed to dispense coordinate system
//     this->rover_position = this->to_rounded_millimeters(drop_center->x);
//     this->max_depth_point_X = this->to_rounded_millimeters(max_depth_point->x);
//     this->max_depth_point_Y = this->to_rounded_millimeters(max_depth_point->y);
//     this->max_Z = this->to_rounded_millimeters(max_depth_point->z);
//     this->Rotate_LFB = Rotate_LFB;
//     this->LFB_Currently_Rotated = LFB_Currently_Rotated;
//     this->Swing_Collision_Expected = Swing_Collision_Expected;
//     this->target_depth_range = target_depth_range;
//     this->target_depth_variance = target_depth_variance;
//     this->interference_max_z = interference_max_z;
//     this->interference_average_z = interference_average_z;
//     this->target_region_max_z = target_region_max_z;
//     Logger::Instance()->Debug("Result in VLS mm coordinates is: X: {}, Y: {}, Z: {}, maxZ: {}", this->rover_position, this->dispense_position, this->depth_result, this->max_Z);
//     this->success_code = success_code;  //0 means success
//     this->request_id = request_id;
//     this->error_description = error_description;
// }
