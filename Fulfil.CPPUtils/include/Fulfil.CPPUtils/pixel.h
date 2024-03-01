#ifndef FULFIL_CPPUTILS_PIXEL_H
#define FULFIL_CPPUTILS_PIXEL_H

namespace fulfil
{
namespace utils
{
/**
 * The purpose of this class is to define a structure
 * to contain the coordinates of a pixel.
 */
class Pixel
{
public:
  /**
   * The x coordinate of the pixel.
   */
  int x;
  /**
   * The y coordinate of the pixel.
   */
  int y;
  /**
   * Pixel constructor
   * @param x coordinate of pixel.
   * @param y coordinate of pixel.
   */
  Pixel(int x, int y);
};
} // namespace utils
} // namespace fulfil
#endif