#include <Fulfil.CPPUtils/depth_pixel.h>

using fulfil::utils::DepthPixel;

DepthPixel::DepthPixel(int x, int y, float depth)
{
    this->x = x;
    this->y = y;
    this->depth = depth; //this is a test
}