#include<memory>
#include <Fulfil.Dispense/drop/drop_result.h>
#include <Fulfil.CPPUtils/logging.h>

using fulfil::dispense::drop::DropResult;
using fulfil::utils::Point3D;
using fulfil::utils::Logger;

float DropResult::to_rounded_millimeters(float mm)
{
  double intermediate = std::round(mm * 1000);
  return intermediate;
}

DropResult::DropResult(std::shared_ptr<std::string> request_id, int error_code)
{
  this->request_id = request_id;
  this->success_code = error_code;
}


DropResult::DropResult(std::shared_ptr<Point3D> drop_center, float max_Z, bool Rotate_LFB, bool LFB_Currently_Rotated,
                       bool Swing_Collision_Expected, std::shared_ptr<std::string> request_id, int success_code)
{
    this->depth_result = this->to_rounded_millimeters(drop_center->z);
    this->dispense_position = this->to_rounded_millimeters(drop_center->y * -1.0); //Notice flip sign so that local bag coordinates transformed to dispense coordinate system
    this->rover_position = this->to_rounded_millimeters(drop_center->x);
    this->max_Z = this->to_rounded_millimeters(max_Z);
    this->Rotate_LFB = Rotate_LFB;
    this->LFB_Currently_Rotated = LFB_Currently_Rotated;
    this->Swing_Collision_Expected = Swing_Collision_Expected;
    Logger::Instance()->Debug("Result in VLS mm coordinates is: X: {}, Y: {}, Z: {}, maxZ: {}", this->rover_position, this->dispense_position, this->depth_result, this->max_Z);
    this->success_code = success_code;  //0 means success
    this->request_id = request_id;
}
