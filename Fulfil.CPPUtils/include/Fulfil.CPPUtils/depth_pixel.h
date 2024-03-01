#ifndef FULFIL_CPPUTILS_DEPTH_PIXEL_H
#define FULFIL_CPPUTILS_DEPTH_PIXEL_H

namespace fulfil
{
namespace utils
{
/**
 * The purpose of this class is to define a simple
 * structure to store data for a pixel that has
 * an associated depth.
 */
class DepthPixel
{
public:
  /**
   * The x coordinate of the pixel location.
   */
  int x;
  /**
   * The y coordinate of the pixel location.
   */
  int y;
  /**
   * The depth at the pixel location.
   */
  float depth;
  /**
   * DepthPixel Constructor
   * @param x coordinate of the pixel location.
   * @param y coordinate of the pixel location.
   * @param depth at the pixel location.
   */
  DepthPixel(int x, int y, float depth);
};
}
}
#endif
