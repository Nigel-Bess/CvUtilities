
#include <gtest/gtest.h>
#include <Fulfil.CPPUtils/point_3d.h>

using std::shared_ptr;
using fulfil::utils::Point3D;
using std::make_shared;
using std::string;

TEST(point3DTests, testToString)
{
    string expected_result("(1.200000, 2.300000, 3.400000)");
    shared_ptr<Point3D> point = make_shared<Point3D>(1.2,2.3,3.4);
    ASSERT_EQ(expected_result, *point->asString());
}