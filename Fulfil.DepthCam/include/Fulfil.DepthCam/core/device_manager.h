/**
 * This file outlines the functionality of the device manager. The purpose of
 * the device manager is to manage the connections between the computer and
 * the realsense cameras. It also provides an abstraction over the realsense cameras
 * by returning the abstract session type.
 */
#ifndef FULFIL_DEPTHCAM_DEVICE_MANAGER_H_
#define FULFIL_DEPTHCAM_DEVICE_MANAGER_H_

#include <Fulfil.DepthCam/core/depth_session.h>

#include<memory>
#include <string>
#include <vector>

#include<librealsense2/rs.hpp>

namespace fulfil
{
namespace depthcam
{
class DeviceManager
{
 private:
  /**
   * The context used to query the connected the devices
   */
  rs2::context context = rs2::context();
  std::vector<rs2::device> managed_devices ;

  bool in_frozen_env;
  /**
    * @purpose Returns a list of serial numbers corresponding to the connected
    * sessions.
    * @return a vector of sessions.
   */
  [[nodiscard]] std::vector<std::string>
    get_device_serials() const;
  /**
   * The list devices that are connected.
   */
  [[nodiscard]] std::vector<rs2::device> get_managed_device_list(std::vector<std::string> greenlit_devices) const;
  [[nodiscard]] std::vector<rs2::device> get_managed_device_list() const;
  [[nodiscard]] std::string get_preset_by_camera_type(std::string_view camera_name) const;

  void reset_context_list();



  // Frozen list of devices

 public:
  /**
   * Basic Constructor.
   */
  DeviceManager();
  DeviceManager(std::vector<std::string> expected_devices, bool frozen);

  virtual ~DeviceManager() = default;

   /**
    * @purpose Attempts to return the session with the given serial number
    * @param serial_number of the desired session.
    * @return the session with the given serial number if it is connected
    * to the device.
    * @throws invalid_argument_exception if no sensor with the given
    * serial number is found.
    * @note This function should be used when you know that a device with
    * a specific serial number is connected and you want to access it. For
    * reasons related to the implementation, it is faster to search through
    * the list of sessions returned by get_connected_sessions to check if
    * the desired one is connected.
    */
  virtual std::shared_ptr<Session> session_by_serial_number(const std::string &serial_number);
   /**
    * @purpose Returns a list of sessions corresponding to the connected
    * devices.
    * @return a vector of sessions.
    */
  virtual std::shared_ptr<std::vector<std::shared_ptr<DepthSession>>> get_connected_sessions();


  /**
   * @purpose Attempts to return the first connected sensor and will retry a
   * specified number of times.
   * @param maximum_tries the number of times the function will try again to
   * find a connected sensor. If the value passed is -1, it will retry forever
   * which could lead to an infinite loop.
   * @return the session corresponding to the first connected sensor
   * @throws runtime_exception when the maximum number of tries is reached
   * and no sensor was found.
   * @note this function is intended as a useful debug method and should
   * be avoided in production code. Use one of the other methods in this
   * class in production.
   */
  std::shared_ptr<Session> get_first_connected_session(int maximum_tries);
};
}  // namespace core
}  // namespace fulfil

#endif  // FULFIL_DEPTHCAM_DEVICE_MANAGER_H_
