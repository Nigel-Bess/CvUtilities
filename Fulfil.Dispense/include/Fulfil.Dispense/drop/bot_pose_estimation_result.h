#pragma once
#include <Fulfil.CPPUtils/math/rigid_transformation.h>

struct BotPoseEstimationResult {
  RigidTransformation Transform;
  float Error;
};