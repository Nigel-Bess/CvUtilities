#include "Fulfil.CPPUtils/eigen/custom_matrix3d_predicate.h"

using fulfil::utils::eigen::CustomMatrix3dPredicate;

CustomMatrix3dPredicate::CustomMatrix3dPredicate(std::function<bool(const fulfil::utils::eigen::Matrix3dPoint &)> predicate)
{
  this->predicate = predicate;
}

bool CustomMatrix3dPredicate::evaluate(const fulfil::utils::eigen::Matrix3dPoint &point)
{
  return this->predicate(point);
}
