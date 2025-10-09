#ifndef FULFIL_CPPUTILS_INCLUDE_FULFIL_CPPUTILS_EIGEN_MATRIX3D_PREDICATE_H_
#define FULFIL_CPPUTILS_INCLUDE_FULFIL_CPPUTILS_EIGEN_MATRIX3D_PREDICATE_H_
#include <eigen3/Eigen/Geometry>

namespace fulfil
{
namespace utils
{
namespace eigen
{
/**
 * This type definition offers an easy shortcut to the somewhat complicated
 * generic type that is retrieved from calling .col(x) on an eigen matrix.
 */
typedef Eigen::Block<Eigen::Matrix<double, 3, -1, 0, 3, -1>, 3, 1, true> Matrix3dPoint;
/**
 * The purpose of this class is to define an interface for predicates that
 * consume an immutable reference to a 3d matrix point and return a boolean.
 */
class Matrix3dPredicate
{
 public:
  /**
   * Returns whether or not the predicate is passed by the given
   * matrix point.
   * @param point immutable reference to a column in a 3d eigen matrix.
   * @return true if it passes the predicate, false otherwise.
   */
  virtual bool evaluate(const Matrix3dPoint& point)=0;

  virtual bool evaluate_side_dispense(const Matrix3dPoint& point) = 0;
};
} // namespace eigen
} // namespace utils
} // namespace fulfils

#endif //FULFIL_CPPUTILS_INCLUDE_FULFIL_CPPUTILS_EIGEN_MATRIX3D_PREDICATE_H_
