/**
 * The purpose of this file is to outline the functionality
 * of some basic visualization tasks related to point clouds.
 */
#ifndef FULFIL_DEPTHCAM_SESSIONVISUALIZER_H
#define FULFIL_DEPTHCAM_SESSIONVISUALIZER_H

#include <memory>
#include <string>

#include <opencv2/aruco/dictionary.hpp>
#include <Fulfil.DepthCam/core/session.h>
#include <Fulfil.CPPUtils/point_3d.h>
#include <Fulfil.DepthCam/aruco.h>

namespace fulfil
{
namespace depthcam
{
namespace visualization
{
class SessionVisualizer
{
 private:
  std::shared_ptr<Session> session;
  std::shared_ptr<std::string> window_name;
  std::shared_ptr<std::pair<int, int>> window_location;
  std::shared_ptr<std::pair<int, int>> window_size;


 public:

  //wait_time for session visualizations is public so it can be changed during different algorithms in drop_zone_searcher
  int wait_time;

  /**
   * The session provides the intrinsics and extrinsics needed to visualize, points used with
   * this class should come from the session that is stored in this class.
   *
   * @param session
   * @param window_name the name of the window when points are displayed.
   */
  /**
   * SessionVisualizer constructor, takes in the session and a name for the window where
   * things will be displayed.
   * @param session where data will be drawn from.
   * @param window_name name of window that where information will be displayed.
   */
  SessionVisualizer(std::shared_ptr<Session> session, std::shared_ptr<std::string> window_name, std::shared_ptr<std::pair <int, int>> window_location, std::shared_ptr<std::pair <int, int>> window_size, int wait_time, bool rgb_base = true);

  /**
   * Displays the RGB image of the session
   */
  void display_rgb_image(std::shared_ptr<cv::Mat> image);

  /**
  * Displays the RGB image of the session, with overlayed central crosshair to assist in camera alignment
  */
  void display_rgb_image_with_crosshair(std::shared_ptr<cv::Mat> image);

  /**
   * Displays the point cloud from the session without applying any transforms.
   *
   * @note this will only make sense if the underlying session is in the camera's coordinate system.
   */
  /**
   * Displays the point cloud that is returned by calling get_point_cloud(should_include_invalid_points)
   * on the inner session.
   * @param should_include_invalid_points if the flag is true, it will include points where the session
   * does not have valid depth data. If the flag is false, it will exclude points where the session does
   * not have valid depth data.
   * @param wait_key the number of milliseconds to wait for a key press before continuing execution
   * of the program. 0 waits infinitely.
   */
  void display_point_cloud(bool should_include_invalid_points);

  /**
   * Displays the given point cloud data onto the rgb stream of the inner session.
   * @param point_cloud a pointer to the point cloud that will be displayed.
   */
  void display_points(std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud,
                      int red = 255,
                      int green = 0,
                      int blue = 0,
                      int radius = 1,
                      int thickness = 5);

  /**
   * Displays the given vector of pixels on the current rbg image of the session.
   * @param pixels a pointer to a vector of pixels that will be displayed.
   * @param wait_key the number of milliseconds to wait for a key press before continuing execution
   * of the program. 0 waits infinitely.
   */
  void display_pixels(std::shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>> pixels,
                      int red = 255,
                      int green = 0,
                      int blue = 0,
                      int radius = 1,
                      int thickness = 5);

  /**
   * Displays a rectangle that is centered around the given point and has the given
   * width and length.
   * @param center a pointer to the center of the rectangle
   * @param width (x-axis) in the camera coordinate system of the rectangle
   * @param length (y-axis) in the camera coordinate system of the rectangle
   * @param wait_key the number of milliseconds to wait for a key press before continuing execution
   * of the program. 0 waits infinitely.
   * @note the length and width are in the camera coordinate system. If the rectangle should
   * be in another coordinate system, use the display_perimeter function.
   */
  void display_rectangle(std::shared_ptr<fulfil::utils::Point3D> center,
      double width, double length);

  /**
   * Draws the markers that were detected on the screen if any, with associated IDs
   * Draws a red box around the region that defines markers that will be used to define the container
   * Draws green circles around valid markers lying in the acceptable region area
   * @param dictionary the dictionary to determine which aruco markers are detected.
   */
  std::shared_ptr<cv::Mat> draw_detected_markers(cv::Ptr<cv::aruco::Dictionary> dictionary, std::shared_ptr<std::vector<std::shared_ptr<fulfil::depthcam::aruco::Marker>>> markers,
                             int region_max_x,
                             int region_min_x,
                             int region_max_y,
                             int region_min_y);
  /**
   * Displays the point cloud where color of points correlates to depth differences.
   * @param point_cloud
   * @param wait_key
   */
  /**
   * Displays the point cloud with colors on the points correlated to the depth value at each pixel.
   * @param point_cloud a pointer to the point cloud that will be displayed.
   * @param wait_key the number of milliseconds to wait for a key press before continuing execution
   * of the program. 0 waits infinitely.
   */
  std::shared_ptr<cv::Mat> display_points_with_depth_coloring(
      std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud);


  std::shared_ptr<cv::Mat> display_points_with_local_depth_coloring(
      std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud,
      std::shared_ptr<cv::Mat> image = nullptr);


  /**
   * Draws lines connecting the lines in given matrix in order of the columns
   * (i.e. line drawn from col 0 to col 1, col 1 to col 2, etc.)
   * @param perimets
   * @param wait_key
   */
  /**
   * Draws lines connecting the points in the given matrix in order of the columns (i.e. line drawn
   * from col 0 to col 1, col 1 to col 2, etc.) on the input image and returns it
   * @param perimeter the point cloud with data that represents the points of the perimeter such that
   * col 0 connects to col 1, col 1 connects to col 2, etc.
   * @param wait_key the number of milliseconds to wait for a key press before continuing execution
   * of the program. 0 waits infinitely.
   * @param color:  1 = green, 2 = red, 3 = blue
   */
  std::shared_ptr<cv::Mat> display_perimeter(std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> perimeter,
                                             std::shared_ptr<cv::Mat> image, int color,
                                             int thickness);

  /**
   * Displays the given series of perimeters.
   * @param perimeters a vector of the perimeters that represent the perimiters to be drawn.
   * @param wait_key the number of milliseconds to wait for a key press before continuing execution
   * of the program. 0 waits infinitely.
   */
  void display_perimeters(std::shared_ptr<std::vector<std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud>>> perimeters);

  /**
 * Displays given image with overlay of pixels highlighted in red that satisfy the pixel intensity requirements
 *
 */
  std::shared_ptr<cv::Mat> display_color_filter(std::shared_ptr<cv::Mat> image, int green_min_intensity, int red_max_intensity, int blue_max_intensity);


  /**
  * Displays given image with overlay of pixels highlighted in red that satisfy the pixel intensity requirements
  *
  */
  std::shared_ptr<cv::Mat> display_HSV_filter(std::shared_ptr<cv::Mat> image);


  /**
  * Displays the provided image
   * @param image to be displayed
  */
  void display_image(std::shared_ptr<cv::Mat> image);

  /**
  * Displays the provided image, but with all of the matrix values shifted and scaled.
   * Lower_input and upper_input define the lower and upper bounds on values in the provided cv::Mat input
   * Lower output and upper output define the desired new bounds that the values will be shifted to (e.g. 0 to 1) for imshow display
  */
  void display_image_normalized(std::shared_ptr<cv::Mat> image, float lower_input, float upper_input,
                                float lower_output, float upper_output);

  /**
   * Applies a false colour map to a single channel depth image and displays it
   * @param image
   */
  void display_depth_image(std::shared_ptr<cv::Mat> image);

};
}  // namespace visualization
}  // namespace core
}  // namespace fulfil



#endif
