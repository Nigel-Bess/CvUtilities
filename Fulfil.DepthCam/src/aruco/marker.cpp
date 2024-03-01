/**
 * This file contains the implementation of the class
 * that represents aruco markers that have been detected.
 */
#include<memory>
#include "Fulfil.DepthCam/aruco/marker.h"
#include"Fulfil.CPPUtils/point_3d.h"
#include <opencv2/opencv.hpp>

using fulfil::depthcam::aruco::Marker;

Marker::Marker(int id, std::shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>> corners)
{
  this->id = id;
  this->corners = corners;
}

int Marker::get_id()
{
  return this->id;
}

std::shared_ptr<cv::Point2f> Marker::get_coordinate(Marker::Coordinate coordinate)
{
  switch(coordinate)
  {
    double x,y,z;
    case Coordinate::topLeft:
      return this->corners->at(0);
    case Coordinate::bottomLeft:
      return this->corners->at(3);
    case Coordinate::topRight:
      return this->corners->at(1);
    case Coordinate::bottomRight:
      return this->corners->at(2);
    case Coordinate::topCenter:
      x = (this->corners->at(0)->x + this->corners->at(1)->x)/2;
      y = (this->corners->at(0)->y + this->corners->at(1)->y)/2;
      return std::make_shared<cv::Point2f>(x,y);
    case Coordinate::bottomCenter:
      x = (this->corners->at(2)->x + this->corners->at(3)->x)/2;
      y = (this->corners->at(2)->y + this->corners->at(3)->y)/2;
      return std::make_shared<cv::Point2f>(x,y);
    case Coordinate::rightCenter:
      x = (this->corners->at(1)->x + this->corners->at(2)->x)/2;
      y = (this->corners->at(1)->y + this->corners->at(2)->y)/2;
      return std::make_shared<cv::Point2f>(x,y);
    case Coordinate::leftCenter:
      x = (this->corners->at(0)->x + this->corners->at(3)->x)/2;
      y = (this->corners->at(0)->y + this->corners->at(3)->y)/2;
      return std::make_shared<cv::Point2f>(x,y);
    case Coordinate::center:
      x = 0;
      y = 0;
      for(int i = 0; i < 4; i++)
      {
        x += this->corners->at(i)->x;
        y += this->corners->at(i)->y;
      }
      return std::make_shared<cv::Point2f>(x / 4, y / 4);
  }
  return std::make_shared<cv::Point2f>();
  // @Steve this could cause some strange deref behavior in a structure
  // I added the return to be safe, but you should confirm this is the desired default behavior
}