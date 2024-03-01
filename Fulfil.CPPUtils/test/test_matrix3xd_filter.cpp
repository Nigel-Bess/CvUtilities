
#include <gtest/gtest.h>
#include <Fulfil.CPPUtils/eigen/matrix3xd_filter.h>
#include <Fulfil.CPPUtils/eigen/custom_matrix3d_predicate.h>
#include <memory>
#include <eigen3/Eigen/Geometry>

using std::shared_ptr;
using std::make_shared;
using fulfil::utils::eigen::Matrix3XdFilter;
using fulfil::utils::eigen::Matrix3dPredicate;
using fulfil::utils::eigen::Matrix3dPoint;
using fulfil::utils::eigen::CustomMatrix3dPredicate;

TEST(testMatrix3xdFilter, testAllFilteredOut)
{
  shared_ptr<Matrix3dPredicate> predicate = make_shared<CustomMatrix3dPredicate>([](const Matrix3dPoint& point) -> bool { return point(2) < 0; });
  shared_ptr<Matrix3XdFilter> filter = make_shared<Matrix3XdFilter>(predicate);
  shared_ptr<Eigen::Matrix3Xd> matrix = make_shared<Eigen::Matrix3Xd>(3, 10);
  for(int i = 0; i < 10; i++)
  {
    (*matrix)(0,i) = 0;
    (*matrix)(1,i) = 0;
    (*matrix)(2,i) = 1;
  }
  ASSERT_EQ(matrix->cols(), 10);
  shared_ptr<Eigen::Matrix3Xd> filtered_mat = filter->filter(matrix);
  ASSERT_EQ(filtered_mat->cols(), 0);
}

TEST(testMatrix3xdFilter, testNoneFilteredOut)
{
  shared_ptr<Matrix3dPredicate> predicate = make_shared<CustomMatrix3dPredicate>([](const Matrix3dPoint& point) -> bool { return point(2) < 0; });
  shared_ptr<Matrix3XdFilter> filter = make_shared<Matrix3XdFilter>(predicate);
  shared_ptr<Eigen::Matrix3Xd> matrix = make_shared<Eigen::Matrix3Xd>(3, 10);
  for(int i = 0; i < 10; i++)
  {
    (*matrix)(0,i) = 0;
    (*matrix)(1,i) = 0;
    (*matrix)(2,i) = -1;
  }
  ASSERT_EQ(matrix->cols(), 10);
  shared_ptr<Eigen::Matrix3Xd> filtered_mat = filter->filter(matrix);
  ASSERT_EQ(filtered_mat->cols(), 10);
}

TEST(testMatrix3xdFilter, testSomeFilteredOut)
{
  shared_ptr<Matrix3dPredicate> predicate = make_shared<CustomMatrix3dPredicate>([](const Matrix3dPoint& point) -> bool { return point(2) < 0; });
  shared_ptr<Matrix3XdFilter> filter = make_shared<Matrix3XdFilter>(predicate);
  shared_ptr<Eigen::Matrix3Xd> matrix = make_shared<Eigen::Matrix3Xd>(3, 10);
  for(int i = 0; i < 5; i++)
  {
    (*matrix)(0,2 * i) = 0;
    (*matrix)(1,2 * i) = 0;
    (*matrix)(2,2 * i) = -1;
    (*matrix)(0, 2 * i + 1) = 0;
    (*matrix)(1, 2 * i + 1) = 0;
    (*matrix)(2, 2 * i + 1) = 1;
  }
  ASSERT_EQ(matrix->cols(), 10);
  shared_ptr<Eigen::Matrix3Xd> filtered_mat = filter->filter(matrix);
  ASSERT_EQ(filtered_mat->cols(), 5);
}