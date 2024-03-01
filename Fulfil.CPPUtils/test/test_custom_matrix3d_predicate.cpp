#include <gtest/gtest.h>
#include <Fulfil.CPPUtils/eigen/custom_matrix3d_predicate.h>
#include <eigen3/Eigen/Geometry>
#include <memory>

using fulfil::utils::eigen::CustomMatrix3dPredicate;
using std::make_shared;
using std::shared_ptr;
using fulfil::utils::eigen::Matrix3dPoint;

TEST(customMatrix3xdPredicateTests,testExecute)
{
  shared_ptr<CustomMatrix3dPredicate> predicate = make_shared<CustomMatrix3dPredicate>([](const Matrix3dPoint& point) -> bool { return point(2) < 0; });
  shared_ptr<Eigen::Matrix3Xd> matrix = make_shared<Eigen::Matrix3Xd>(3,2);
  (*matrix)(0,0) = 0;
  (*matrix)(1,0) = 0;
  (*matrix)(2,0) = -1;
  (*matrix)(0,1) = 0;
  (*matrix)(1,1) = 0;
  (*matrix)(2,1) = 1;
  ASSERT_TRUE(predicate->evaluate(matrix->col(0)));
  ASSERT_FALSE(predicate->evaluate(matrix->col(1)));
}