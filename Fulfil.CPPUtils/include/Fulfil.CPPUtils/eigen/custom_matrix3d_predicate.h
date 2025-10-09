#ifndef FULFIL_CPPUTILS_INCLUDE_FULFIL_CPPUTILS_EIGEN_CUSTOM_MATRIX3D_PREDICATE_H_
#define FULFIL_CPPUTILS_INCLUDE_FULFIL_CPPUTILS_EIGEN_CUSTOM_MATRIX3D_PREDICATE_H_
#include <Fulfil.CPPUtils/eigen/matrix3d_predicate.h>

namespace fulfil
{
namespace utils
{
namespace eigen
{
/**
 * The purpose of this class is to define a simple 3d matrix predicate
 * that just stores a pointer to a function containing the implementation
 * of the predicate. This allows for a lightweight way to create predicates
 * since you don't have to create a new class.
 */
class CustomMatrix3dPredicate : public Matrix3dPredicate
{
 private:
  /**
   * The inner function that returns a bool and takes in a 3d matrix point.
   */
  std::function<bool (const fulfil::utils::eigen::Matrix3dPoint&)> predicate;
 public:
  /**
   * CustomMatrix3dPredicate Constructor
   * @param predicate a functor that returns a bool and takes in a 3d
   * matrix point.
   */
  explicit CustomMatrix3dPredicate(std::function<bool (const fulfil::utils::eigen::Matrix3dPoint&)> predicate);

  bool evaluate(const Matrix3dPoint &point) override;

  bool evaluate_side_dispense(const Matrix3dPoint& point) override;
};
} // namespace eigen
} // namespace utils
} // namespace fulfil


#endif //FULFIL_CPPUTILS_INCLUDE_FULFIL_CPPUTILS_EIGEN_CUSTOM_MATRIX3D_PREDICATE_H_
