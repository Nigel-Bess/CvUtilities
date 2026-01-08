#pragma once
#include <Fulfil.CPPUtils/json/nlohmann_serializers.h>

struct ArucoTag {
  Eigen::Vector3d Position;
  int Id;
  
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ArucoTag, Position, Id)
