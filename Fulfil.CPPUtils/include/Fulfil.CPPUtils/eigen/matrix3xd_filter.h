#ifndef FULFIL_CPPUTILS_INCLUDE_FULFIL_CPPUTILS_EIGEN_MATRIX3XD_FILTER_H_
#define FULFIL_CPPUTILS_INCLUDE_FULFIL_CPPUTILS_EIGEN_MATRIX3XD_FILTER_H_
#include <Fulfil.CPPUtils/eigen/matrix3d_predicate.h>
#include <memory>

namespace fulfil
{
namespace utils
{
namespace eigen
{
/**
 * This class serves to create a new matrix from a subset of
 * points in another matrix based on the predicate provided by
 * the user.
 */
class Matrix3XdFilter
{
 private:
  /**
   * pointer to the predicate that will be used to determine which columns of the matrix
   * are added to the new filtered matrix.
   */
  std::shared_ptr<fulfil::utils::eigen::Matrix3dPredicate> predicate;
 public:
  /**
   * Matrix3XdFilter Constructor
   * @param predicate pointer to the predicate that will be used to determine which columns
   * of the matrix are added to a new filtered matrix.
   */
  Matrix3XdFilter(std::shared_ptr<fulfil::utils::eigen::Matrix3dPredicate> predicate);
  /**
   * Returns a new matrix with just the columns that caused the predicate to return true.
   * @param matrix pointer to matrix data to filter.
   * @return a pointer to a new matrix with just data where the predicate returned true.
   */
  std::shared_ptr<Eigen::Matrix3Xd> filter(std::shared_ptr<Eigen::Matrix3Xd> matrix);

  std::shared_ptr<Eigen::Matrix3Xd> filter_side_dispense(std::shared_ptr<Eigen::Matrix3Xd> matrix);
  
  std::shared_ptr<Eigen::Matrix3Xd> filter_side_dispense_point_cloud_outside_cavity(std::shared_ptr<Eigen::Matrix3Xd> matrix);
};
} // namespace eigen
} // namespace utils
} // namespace fulfil
#endif //FULFIL_CPPUTILS_INCLUDE_FULFIL_CPPUTILS_EIGEN_MATRIX3XD_FILTER_H_
