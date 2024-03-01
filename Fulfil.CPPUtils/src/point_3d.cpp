#include "Fulfil.CPPUtils/point_3d.h"
using fulfil::utils::Point3D;

Point3D::Point3D(float x, float y, float z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

std::shared_ptr<std::string> Point3D::asString()
{
    std::shared_ptr<std::string> result = std::make_shared<std::string>();
    result->append("(");
    result->append(std::to_string(this->x));
    result->append(", ");
    result->append(std::to_string(this->y));
    result->append(", ");
    result->append(std::to_string(this->z));
    result->append(")");
    return result;
}

std::ostream& operator<<(std::ostream& os, const Point3D& pt)
{
    os << "(" << pt.x << ", " << pt.y << ", " << pt.z << ")";
    return os;
}
fulfil::utils::Point3D::Point3D(const fulfil::utils::eigen::Matrix3dPoint &point)
{
  this->x = point(0);
  this->y = point(1);
  this->z = point(2);
}
