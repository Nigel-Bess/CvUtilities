#ifndef STATS_HELPER_DELEGATES_H_
#define STATS_HELPER_DELEGATES_H_
#include "point_3d.h"
#include "stats_helper.h"
#include <cmath>

namespace fulfil
{
namespace utils
{
/**
 *  A stat delegate for 3DPoint. Basically just treats the 3DPoint as a tuple and collects stats on all of the x's,
 *  y's, and z's.
 */
 class Point3DStatDelegate : public StatsHelperDelegate<std::shared_ptr<fulfil::utils::Point3D>>, public std::enable_shared_from_this<Point3DStatDelegate>
{
  std::shared_ptr<fulfil::utils::Point3D> add(std::shared_ptr<fulfil::utils::Point3D> a, std::shared_ptr<fulfil::utils::Point3D> b) override
  {
    return std::make_shared<fulfil::utils::Point3D>(a->x + b->x, a->y + b->y, a->z + b->z);
  }
  std::shared_ptr<fulfil::utils::Point3D> divide(std::shared_ptr<fulfil::utils::Point3D> a, int b) override
  {
    return std::make_shared<fulfil::utils::Point3D>(a->x / b, a->y / b, a->z / b);
  }
  std::shared_ptr<fulfil::utils::Point3D> power(std::shared_ptr<fulfil::utils::Point3D> a, double exponent) override
  {
    return std::make_shared<fulfil::utils::Point3D>(std::pow(a->x, exponent), std::pow(a->y, exponent), std::pow(a->z, exponent));
  }
  std::shared_ptr<fulfil::utils::Point3D> subtract(std::shared_ptr<fulfil::utils::Point3D> a, std::shared_ptr<fulfil::utils::Point3D> b) override
  {
    return std::make_shared<fulfil::utils::Point3D>(a->x - b->x, a->y - b->y, a->z - b->z);
  }
  std::shared_ptr<fulfil::utils::Point3D> sum_base() override
  {
    return std::make_shared<fulfil::utils::Point3D>(0,0,0);
  }
  std::shared_ptr<fulfil::utils::Point3D> sqrt(std::shared_ptr<fulfil::utils::Point3D> a) override
  {
    return std::make_shared<fulfil::utils::Point3D>(std::sqrt(a->x), std::sqrt(a->y), std::sqrt(a->z));
  }
  std::shared_ptr<fulfil::utils::Point3D> max(std::shared_ptr<fulfil::utils::Point3D> a, std::shared_ptr<fulfil::utils::Point3D> b) override
  {
    return std::make_shared<fulfil::utils::Point3D>(std::max(a->x, b->x), std::max(a->y, b->y), std::max(a->z, b->z));
  }
  std::shared_ptr<fulfil::utils::Point3D> min(std::shared_ptr<fulfil::utils::Point3D> a, std::shared_ptr<fulfil::utils::Point3D> b) override
  {
    return std::make_shared<fulfil::utils::Point3D>(std::min(a->x, b->x), std::min(a->y, b->y), std::min(a->z, b->z));
  }
};

/**
 * A stat delegate for floats.
 */
 class FloatStatDelegate : public StatsHelperDelegate<float>, public std::enable_shared_from_this<FloatStatDelegate>
{
  float add(float a, float b)
  {
    return a + b;
  }
  float divide(float a, int b)
  {
    return a / b;
  }
  float power(float a, double exponent)
  {
    return pow(a,exponent);
  }
  float subtract(float a, float b)
  {
    return a-b;
  }
  float sum_base()
  {
    return 0;
  }
  float sqrt(float a)
  {
    return std::sqrt(a);
  }
  float max(float a, float b)
  {
    return std::max(a, b);
  }
  float min(float a, float b)
  {
    return std::min(a, b);
  }
};

/**
 * A stat delegate for floats.
 */
 class DoubleStatDelegate : public StatsHelperDelegate<double>, public std::enable_shared_from_this<DoubleStatDelegate>
{
  double add(double a, double b)
  {
    return a + b;
  }
  double divide(double a, int b)
  {
    return a / b;
  }
  double power(double a, double exponent)
  {
    return pow(a,exponent);
  }
  double subtract(double a, double b)
  {
    return a-b;
  }
  double sum_base()
  {
    return 0;
  }
  double sqrt(double a)
  {
    return std::sqrt(a);
  }
  double max(double a, double b)
  {
    return std::max(a, b);
  }
  double min(double a, double b)
  {
    return std::min(a, b);
  }
};
}
}
#endif