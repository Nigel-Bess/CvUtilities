#ifndef FULFIL_CPPUTILS_POINT_3D_H
#define FULFIL_CPPUTILS_POINT_3D_H
#include <cstring>
#include <iostream>
#include <sstream>
#include <memory>
#include <Fulfil.CPPUtils/eigen/matrix3d_predicate.h>

namespace fulfil
{
namespace utils
{
/**
 * The purpose of this struct is to store information about
 * 3d points.
 */
struct Point3D
{
  /**
   * x coordinate of the point.
   */
  float x;
  /**
   * y coordinate of the point.
   */
  float y;
  /**
   * z coordinate of the point.
   */
  float z;
  /**
   * Point3D constructor.
   * @param x coordinate of point.
   * @param y coordinate of point.
   * @param z coordinate of point.
   */
  Point3D(float x, float y, float z);
  /**
   * Point3D constructor that takes in a matrix point for convenience
   * @param point constant reference to the matrix point.
   */
  Point3D(const fulfil::utils::eigen::Matrix3dPoint& point);
  /**
   * Returns a string encoding of the point. Useful for debugging.
   * Format is (x, y, z)
   * @return pointer to string containing encoding.
   */
  std::shared_ptr<std::string> asString();
  /**
   * Writes a formatted data representing the point to the given
   * os stream
   * @param os output stream where encoded data will be written.
   * @param pt the point that will be written.
   * @return the output stream.
   */
  friend std::ostream& operator<<(std::ostream& os, const Point3D& pt);
};
} // namespace util
} // namespace fulfil
#endif
