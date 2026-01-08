#pragma once
#include <array>
#include <Fulfil.CPPUtils/json/nlohmann_serializers.h>
#include "aruco_tag.h"

struct ArucoTagMatch {
  ArucoTag TagDefinitionAtIdenityTransform;
  Eigen::Vector3d MeasuredPosition;  
};