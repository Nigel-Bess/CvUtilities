//
// Created by nkaffine on 12/2/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains the implementation of a container that
 * calculates the transformation to local point cloud dynamically
 * using aruco markers.
 */
#include <librealsense2/rsutil.h>
#include <Fulfil.CPPUtils/eigen/matrix3xd_filter.h>
#include <Fulfil.CPPUtils/logging.h>
#include "Fulfil.DepthCam/aruco/kabsch_helper.h"
#include <Fulfil.DepthCam/aruco/marker_detector_container.h>
#include <Fulfil.DepthCam/point_cloud.h>
#include <Fulfil.DepthCam/point_cloud/local_point_cloud.h>
#include "container_matrix3xd_predicate.h"

using fulfil::depthcam::aruco::ContainerMatrix3xdPredicate;
using fulfil::depthcam::aruco::Marker;
using fulfil::depthcam::aruco::MarkerDetector;
using fulfil::depthcam::aruco::MarkerDetectorContainer;
using fulfil::depthcam::pointcloud::LocalPointCloud;
using fulfil::depthcam::pointcloud::PointCloud;
using fulfil::depthcam::Session;
using fulfil::utils::commands::dc_api_error_codes::DcApiError;
using fulfil::utils::commands::dc_api_error_codes::DcApiErrorCode;
using fulfil::utils::eigen::Matrix3XdFilter;
using fulfil::utils::eigen::Matrix3dPoint;
using fulfil::utils::Logger;

MarkerDetectorContainer::MarkerDetectorContainer(std::shared_ptr<MarkerDetector> marker_detector,
                                                 std::shared_ptr<Session> session,
                                                 bool should_filter_points_outside_of_container,
                                                 bool is_side_dispense,
                                                 bool extend_region_over_markers,
                                                 float width,
                                                 float length,
                                                 float outer_width,
                                                 float outer_length,
                                                 std::shared_ptr<std::vector<Marker::Coordinate>> corners,
                                                 std::shared_ptr<Eigen::Matrix3Xd> mm_marker_coordinates,
                                                 int num_markers,
                                                 float marker_depth,
                                                 float marker_depth_tolerance,
                                                 int min_marker_count_for_validation,
                                                 int region_max_x,
                                                 int region_min_x,
                                                 int region_max_y,
                                                 int region_min_y)
{
  this->marker_detector = marker_detector;
  this->session = session;
  this->is_side_dispense = is_side_dispense;
  this->width = width;
  this->length = length;
  this->outer_width = outer_width;
  this->outer_length = outer_length;
  this->should_filter_points_outside_of_container = should_filter_points_outside_of_container;
  this->extend_region_over_markers = extend_region_over_markers;
  this->corners = corners;
  this->mm_marker_coordinates = mm_marker_coordinates;
  this->num_markers = num_markers;
  this->marker_depth = marker_depth;
  this->marker_depth_tolerance = marker_depth_tolerance;
  this->min_marker_count_for_validation = min_marker_count_for_validation;
  this->region_min_x = region_min_x;
  this->region_max_x = region_max_x;
  this->region_min_y = region_min_y;
  this->region_max_y = region_max_y;
}

/**
 * Converts the given color stream pixel to a depth stream point.
 */
std::shared_ptr<fulfil::utils::Point3D> convert_color_pixel_to_depth_point(float x, float y, std::shared_ptr<Session> session)
{
  //Converting color pixel to color point
  float color_point[3];
  const float pixel[2] = {x, y};
  float depth = session->depth_at_pixel(round(x), round(y));  //get depth data at input pixel (x,y) in camera coordinates, using realsense library function
  std::shared_ptr<std::vector<std::shared_ptr<std::pair<std::shared_ptr<cv::Point2f>, float>>>> point =
     std::make_shared<std::vector<std::shared_ptr<std::pair<std::shared_ptr<cv::Point2f>, float>>>>();
  point->push_back(std::make_shared<std::pair<std::shared_ptr<cv::Point2f>,float>>(std::make_shared<cv::Point2f>(x,y), depth));
  std::shared_ptr<PointCloud> cloud = session->get_point_cloud(true, __FUNCTION__)->as_pixel_cloud()->new_point_cloud(point);
  std::shared_ptr<Eigen::Matrix3Xd> data = cloud->as_camera_cloud()->get_data();
  //Return the new point
  return std::make_shared<fulfil::utils::Point3D>((*data)(0,0), (*data)(1,0), (*data)(2,0));
}

cv::Mat MarkerDetectorContainer::grab_color_frame() {
    return this->session->grab_color_frame();
}

/**
 * Calculates the eigen transformation that will convert points from the depth stream's
 * coordinate system to points in the bag's coordinate system.
 */
std::shared_ptr<Eigen::Affine3d> MarkerDetectorContainer::get_transform_to_bag_coordinates(
    std::shared_ptr<std::vector<std::shared_ptr<Marker>>> detected_markers)
{
    std::shared_ptr<Eigen::Matrix3Xd> local_marker_coordinates = this->mm_marker_coordinates;
    int num_detected_markers = detected_markers->size();
    auto validate_against_marker_set = [](int detected_marker, int marker1, int marker2, int marker3, int marker4) {
        return (detected_marker == marker1 or detected_marker == marker2 or detected_marker == marker3 or detected_marker == marker4);
    };

    bool front_side_has_a_marker =  std::any_of(detected_markers->cbegin(), detected_markers->cend(), [&validate_against_marker_set](auto marker_ptr)
      { return validate_against_marker_set(marker_ptr->get_id(), 1,2,3,4); } );
    bool back_side_covered_has_a_marker =  std::any_of(detected_markers->cbegin(), detected_markers->cend(), [&validate_against_marker_set](auto marker_ptr)
      { return validate_against_marker_set(marker_ptr->get_id(), 0,7,6,5); } );

    /**
    * Creating two matrices that represent a mapping between the coordinate system of the bag and the coordinate system
    * of the camera. Columns of the two matrices correspond.
    */
    std::shared_ptr<Eigen::Matrix3Xd> local_marker_coordinates_2 = std::make_shared<Eigen::Matrix3Xd>(3, num_detected_markers);
    std::shared_ptr<Eigen::Matrix3Xd> camera_coordinates = std::make_shared<Eigen::Matrix3Xd>(3, num_detected_markers);

    for(int i = 0; i < num_detected_markers; i++)    //iterate over the detected valid markers vector (not in order of IDs)
    {
      int id = detected_markers->at(i)->get_id();

      (*local_marker_coordinates_2)(0, i) = (*local_marker_coordinates)(0, id); //copy over bag mm coordinates for the detected marker
      (*local_marker_coordinates_2)(1, i) = (*local_marker_coordinates)(1, id);
      (*local_marker_coordinates_2)(2, i) = (*local_marker_coordinates)(2, id);

      std::shared_ptr<cv::Point2f> p2d = detected_markers->at(i)->get_coordinate(this->corners->at(id)); //gets pixel coordinate at specified corner position for the detected marker
      fulfil::utils::Point3D p = *convert_color_pixel_to_depth_point(p2d->x, p2d->y, this->session); //gets x, y, and depth coordinate in camera coordinate system
      (*camera_coordinates)(0, i) = p.x;
      (*camera_coordinates)(1, i) = p.y;
      (*camera_coordinates)(2, i) = p.z; //1.0 + i * 0.0001 //Todo: can use this section to modify transformation depths if needed

      if(i == num_detected_markers - 1)
      {
        if(!front_side_has_a_marker or !back_side_covered_has_a_marker)
        {
          Logger::Instance()->Warn("One side of markers is fully missing. Transformation should still work.");
          /**
          (*local_marker_coordinates_2)(1, i) += this->marker_adjust_amount;

          //this calculation tests for the orientation of the camera relative to the LFB
          bool adjust_type_2 =  abs((*camera_coordinates)(0, 0) - (*camera_coordinates)(0, 1)) > abs((*camera_coordinates)(1, 0) - (*camera_coordinates)(1, 1));
          if(adjust_type_2)
          {
            (*camera_coordinates)(1, i) -=  this->marker_adjust_amount; //in this case, camera coordinate Y changes in opposite direction as LFB coordinate Y
          }
          else
          {
            (*camera_coordinates)(0, i) +=  this->marker_adjust_amount; //in this case, camera coordinate X changes in same direction as LFB coordinate Y
          }
           **/
        }
      }
    }

    KabschHelper helper;

    std::shared_ptr<Eigen::Affine3d> transform = helper.find_translation_between_points(camera_coordinates,
                                                                                        local_marker_coordinates_2);
    return transform;
}

std::vector<std::shared_ptr<Marker>> MarkerDetectorContainer::marker_selection(std::vector<std::shared_ptr<Marker>> parallel_markers) {
    int counter = 0;
    std::vector<std::shared_ptr<Marker>> choosen_markers;
    for (int i = 0; i < parallel_markers.size(); ++i) {
        for (int j = i + 1; j < parallel_markers.size(); ++j) {
            if (parallel_markers[i]->get_id() != parallel_markers[j]->get_id()) {
                if ((abs(parallel_markers[i]->get_coordinate(Marker::Coordinate::center)->x - parallel_markers[j]->get_coordinate(Marker::Coordinate::center)->x) < 500) &&
                    (abs(parallel_markers[i]->get_coordinate(Marker::Coordinate::center)->x - parallel_markers[j]->get_coordinate(Marker::Coordinate::center)->x) > 300) &&
                    (abs(parallel_markers[i]->get_coordinate(Marker::Coordinate::center)->y - parallel_markers[j]->get_coordinate(Marker::Coordinate::center)->y) < 100)) {
                    choosen_markers.push_back(parallel_markers[i]);
                    choosen_markers.push_back(parallel_markers[j]);
                    counter++;
                }
            }
            if (counter == 1) break;
        }
    }
    return choosen_markers;
}

std::shared_ptr<std::vector<std::shared_ptr<Marker>>> MarkerDetectorContainer::validate_markers(std::shared_ptr<std::vector<std::shared_ptr<Marker>>> markers, bool bot_in_nominal_orientation)
{

    Logger::Instance()->Debug("Validating marker detections now on {} bot", bot_in_nominal_orientation ? "nominal" : "rotated");

    //check that markers are within search zone and at expected depth
    auto is_on_dispense_right = [&bot_in_nominal_orientation](int detected_marker) {
        if (bot_in_nominal_orientation) {
            return (detected_marker == 0 or detected_marker == 5 or detected_marker == 6 or detected_marker == 7);
        }
        return detected_marker != 0 && detected_marker != 5 && detected_marker != 6 && detected_marker != 7;

    };
    auto get_record = [](std::vector<std::shared_ptr<Marker>>& mrkrs, std::string_view const rem)
    {
        std::stringstream logstr;
        for (auto a : mrkrs)
            logstr << "(id: " << a->get_id()
            << ", x: " << a->get_coordinate(Marker::Coordinate::center)->x
            << ", y: " << a->get_coordinate(Marker::Coordinate::center)->y << ") ";
        Logger::Instance()->Info("Markers->\n    {} :: {}", logstr.str(), rem);
    };
    get_record(*markers, "Before marker sort and depth filter");
    //vector to store potential parallel marker pairs
    std::vector<std::vector<std::shared_ptr<Marker>>> potential_markers(4);
    std::vector<std::shared_ptr<Marker>> good_markers;
    std::vector<std::shared_ptr<Marker>> true_markers;
    for (int i = 0; i < markers->size(); i++)
    {
        std::shared_ptr<Marker> a = markers->at(i);
        
        if (a->get_id() == 0 || a->get_id() == 1) {
            potential_markers[0].push_back(a);
        }
        else if (a->get_id() == 2 || a->get_id() == 7) {
            potential_markers[1].push_back(a);
        }
        else if (a->get_id() == 3 || a->get_id() == 6) {
            potential_markers[2].push_back(a);
        }
        else if (a->get_id() == 4 || a->get_id() == 5) {
            potential_markers[3].push_back(a);
        }
    }
    Logger::Instance()->Debug("Calling marker_selection : Choosing the 8 unique markers and filling the vector");
    //choosing valid markers from the potential markers
    for (int i = 0; i < potential_markers.size(); i++) {
        good_markers = marker_selection(potential_markers[i]);
        true_markers.insert(true_markers.end(), good_markers.begin(), good_markers.end());
    }
    markers = std::make_shared<std::vector<std::shared_ptr<Marker>>>(true_markers);
    Logger::Instance()->Debug("Choosen marker size: " + std::to_string(markers->size()));
  for(int i = 0; i < markers->size(); i++)
  {
    std::shared_ptr<Marker> marker = markers->at(i);

    int id = markers->at(i)->get_id();
    float marker_x = marker->get_coordinate(this->corners->at(id))->x;
    float marker_y = marker->get_coordinate(this->corners->at(id))->y;

    //validate marker is at expected depth from camera
    float detected_depth = this->camera_depth_at_pixel(round(marker_x), round(marker_y)); //This rounding matches how the code creates the transform to bag coordinates in marker_detector_container.cpp
    if(abs(detected_depth - this->marker_depth) > this->marker_depth_tolerance)
    {
      Logger::Instance()->Warn("Marker ID {} at ({},{})  had detected depth outside of acceptable range and was removed. Detected: {}, expected: {}, tolerance: {}",
                               marker->get_id(), marker_x, marker_y, detected_depth, this->marker_depth, this->marker_depth_tolerance);
      markers->erase(markers->begin() + i);
      i--;
    }
  }
  Logger::Instance()->Debug("Number of valid markers after first round of checks: {}", markers->size());


  std::sort(markers->begin(), markers->end(), [&is_on_dispense_right](auto a, auto b) {
      if (a->get_id() < b->get_id()) { return true; }
      if (a->get_id()== b->get_id() &&  !is_on_dispense_right(a->get_id()) &&
          a->get_coordinate(Marker::Coordinate::center)->x <
            b->get_coordinate(Marker::Coordinate::center)->x) { return true; }
      if (a->get_id()== b->get_id() && is_on_dispense_right(a->get_id()) &&
          a->get_coordinate(Marker::Coordinate::center)->x >
            b->get_coordinate(Marker::Coordinate::center)->x) { return true; }
      return false;
  });
  get_record(*markers, "AFTER marker sort and depth filter");
  /**
   *  Remove duplicates
   */

  //copy over non-duplicate markers to final_markers vector
  std::vector<std::shared_ptr<Marker>> final_markers;
  std::unique_copy(markers->begin(), markers->end(), std::back_inserter(final_markers),
    [](auto a, auto b) { return a->get_id() == b->get_id(); });
  get_record(final_markers, "After removing overlapping markers");

  Logger::Instance()->Debug("Number of valid markers after second round of checks: {}", final_markers.size());
  if (final_markers.size() > num_markers)  Logger::Instance()->Error("Too many valid markers! Something went wrong");
  Logger::Instance()->Trace("returning valid markers");
  return std::make_shared<std::vector<std::shared_ptr<Marker>>>(final_markers);
}

std::shared_ptr<std::vector<std::shared_ptr<Marker>>> MarkerDetectorContainer::validate_markers_side_dispense(std::shared_ptr<std::vector<std::shared_ptr<Marker>>> markers)
{
  Logger::Instance()->Debug("Validating marker detections now");

  auto get_record = [](std::vector<std::shared_ptr<Marker>> &mrkrs, std::string_view const rem)
  {
      std::stringstream logstr ;
      for (auto a : mrkrs)
          logstr << "(id: " << a->get_id()
                 << ", x: " << a->get_coordinate(Marker::Coordinate::center)->x
                 << ", y: " << a->get_coordinate(Marker::Coordinate::center)->y << ") ";
      Logger::Instance()->Info("Markers->\n    {} :: {}",logstr.str(), rem);
  };

  //check that markers are within search zone and at expected depth
  auto is_on_bot_left = [](int detected_marker) {
    return (detected_marker == 6 or detected_marker == 2 or detected_marker == 1 or detected_marker == 3);
  };

  auto is_on_bot_top = [](int detected_marker) {
    return (detected_marker == 0 or detected_marker == 7 or detected_marker == 6); // TODO add the mysterious marker
  };

  /*
  1. if marker is on left or on top:
        make sure it is in the expected range if the bot is centered on image
        TODO: how to prevent bots in the background 
  2. else: reject
  */

  get_record(*markers, "Before marker sort and depth filter");

  for(int i = 0; i < markers->size(); i++)
  {
    bool erase = false;
    std::shared_ptr<Marker> marker = markers->at(i);

    int id = markers->at(i)->get_id();
    float marker_x = marker->get_coordinate(this->corners->at(id))->x;
    float marker_y = marker->get_coordinate(this->corners->at(id))->y;

    if (!is_on_bot_left(id) && !is_on_bot_top(id)) { 
      Logger::Instance()->Warn("Marker ID {} at ({},{}) : Not on bot left or bot top, and was removed",
                               marker->get_id(), marker_x, marker_y);
      erase = true;
    }
    //validate marker is at expected depth from camera
    float detected_depth = this->camera_depth_at_pixel(round(marker_x), round(marker_y)); //This rounding matches how the code creates the transform to bag coordinates in marker_detector_container.cpp
    if(abs(detected_depth - this->marker_depth) > this->marker_depth_tolerance)
    {
      Logger::Instance()->Warn("Marker ID {} at ({},{})  had detected depth outside of acceptable range and was removed. Detected: {}, expected: {}, tolerance: {}",
                               marker->get_id(), marker_x, marker_y, detected_depth, this->marker_depth, this->marker_depth_tolerance);
      erase = true;
    }
    if (erase) {
      markers->erase(markers->begin() + i);
      i--;
    }
  }
  Logger::Instance()->Debug("Number of valid markers after first round of checks: {}", markers->size());

  std::sort(markers->begin(), markers->end(), [&is_on_bot_left, &is_on_bot_top](auto a, auto b) {
      // if a's ID is less than b's ID
      if (a->get_id() < b->get_id()) { return true; }
      // if a & b are same ID, on bot left and a is farther left than b
      if (a->get_id() == b->get_id() && is_on_bot_left(a->get_id()) &&
          a->get_coordinate(Marker::Coordinate::center)->x <
            b->get_coordinate(Marker::Coordinate::center)->x) { return true; }
      // if a & b are same ID, on bot top, and a is higher than b
      if (a->get_id() == b->get_id() && is_on_bot_top(a->get_id()) &&
          a->get_coordinate(Marker::Coordinate::center)->y <
            b->get_coordinate(Marker::Coordinate::center)->y) { return true; }
      return false;
  });
  get_record(*markers, "AFTER marker sort and depth filter");
  /**
   *  Remove duplicates
   */

  // copy over non-duplicate markers to final_markers vector
  std::vector<std::shared_ptr<Marker>> final_markers;
  std::unique_copy(markers->begin(), markers->end(), std::back_inserter(final_markers),
    [](auto a, auto b) { return a->get_id() == b->get_id(); });
  get_record(final_markers, "After removing overlapping markers");

  Logger::Instance()->Debug("Number of valid markers after second round of checks: {}", final_markers.size());
  if (final_markers.size() > num_markers)  Logger::Instance()->Error("Too many valid markers! Something went wrong");
  Logger::Instance()->Trace("returning valid markers");
  return std::make_shared<std::vector<std::shared_ptr<Marker>>>(final_markers);
}

/**
 * Returns a list of all of the detected markers with nullptr where no markers were detected. The id of the detected
 * marker determines where in the array it is placed.
 * @param marker_detector the marker detector to use when search for detectors
 * @param depth_session the depth session to use for detecting markers.
 * @return a list of length(num_markers) either nullptr or markers representing the spots on the bag with the markers.
 */
std::shared_ptr<std::vector<std::shared_ptr<Marker>>> MarkerDetectorContainer::get_markers(bool bot_in_nominal_orientation)
{
  if(this->cached_markers != nullptr)
  {
    return this->cached_markers;
  }
  else
  {
    std::shared_ptr<Session> session = this->session;
    std::shared_ptr<std::vector<std::shared_ptr<Marker>>> detected_markers = marker_detector->detect_markers(session->get_color_mat());
    std::shared_ptr<std::vector<std::shared_ptr<Marker>>> valid_markers = (this->is_side_dispense) ? 
                                                                          validate_markers_side_dispense(detected_markers) : 
                                                                          validate_markers(detected_markers, bot_in_nominal_orientation); //validates depth and within search region

    this->cached_markers = valid_markers; //cache the markers for use in visualizations, etc. without needing to re-detect
    return valid_markers;
  }
}

/**
 * Returns whether the given point (from the bag coordinate system) is inside the bag.
 * @param point a reference to the point in bag coordinates.
 * @param bag_dimensions a reference to the point with the dimensions of the bag.
 * @return true if the point is in the bag, false otherwise.
 */
bool point_in_bag(std::shared_ptr<Matrix3dPoint> point, const float& width, const float& length)
{
    return (*point)(0) >= -width/2
           && (*point)(0) <= width/2
           && (*point)(1) >= -length/2
           && (*point)(1) <= length/2;
}

void MarkerDetectorContainer::set_service(std::shared_ptr<GrpcService> serv){
    this->session->set_service(serv);
}

void MarkerDetectorContainer::setup_cached_container()
{
  Logger::Instance()->Debug("Setup Cached Container in marker_detector_container");

  std::shared_ptr<std::vector<std::shared_ptr<Marker>>> valid_markers = get_markers();

  int num_detections = valid_markers->size();
  Logger::Instance()->Debug("Detected number of valid markers: {}", num_detections);

  if(num_detections < this->min_marker_count_for_validation) // must have at least 3 detected markers in order for coordinate transform to work properly.
  {
    if(num_detections == 0)
    {
      Logger::Instance()->Error("No Valid Markers Found; Cam: LFB");
      throw DcApiError(DcApiErrorCode::NoMarkersDetected, std::string("No markers detected"));
    }
    else
    {
      Logger::Instance()->Error("Not Enough Valid Markers Found; Cam: LFB");
      // TODO improve error description to take in minimum amount of markers config value (in a future PR)
      std::string error_descrip = "Number of markers detected: " + std::to_string(num_detections) +
              ", but minimum amount of markers needed: " + std::to_string(this->min_marker_count_for_validation) + 
              ", out of total number of markers: " + std::to_string(num_markers);
      throw DcApiError(DcApiErrorCode::NotEnoughMarkersDetected, error_descrip);
    }
  }

  /** WARNING: see notes in marker_detector_container.h helper file
   *  For now, this boolean changes nothing
   *  TODO: restructure code to handle this differently, without risk of mistakenly passing around cached expanded-region containers
   *  TODO: the extend values may need to be changed in the future if LFR gets closer to DAB / dispense and leads to mistaken detections
   */
  if (this->extend_region_over_markers)
  {
    this->length = this->outer_length;
    this->width = this->outer_width;
    Logger::Instance()->Debug("Container logic has opted in to container region extension over markers. The new MarkerDetectorContainer length is {} and the width is {}", this->outer_length, this->outer_width);
    //Logger::Instance()->Error("CONTAINER REGION EXTENSION FUNCTIONALITY NEEDS REFACTORING. EXTENSION NOT EXECUTED!!!");
  }

  Logger::Instance()->Debug("Getting transform now in setup_cached_container");
  std::shared_ptr<Eigen::Affine3d> transform = this->get_transform_to_bag_coordinates(valid_markers);
  this->cached_container =std::make_shared<fulfil::depthcam::aruco::FixedTransformContainer>(this->session,
      transform, this->should_filter_points_outside_of_container,
      this->width, this->length);
}

void MarkerDetectorContainer::lock()
{
    this->session->lock();
}

void MarkerDetectorContainer::unlock()
{
    this->session->unlock();
}

std::shared_ptr<std::string> MarkerDetectorContainer::get_serial_number()
{
    return this->session->get_serial_number();
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> MarkerDetectorContainer::get_point_cloud(
    bool include_invalid_depth_data, const char* caller)
{
  Logger::Instance()->Info("MarkerDetectorContainer.get_point_cloud called by " + std::string(caller));
  if(!this->cached_container)
  {
    this->setup_cached_container();
  }
  return this->cached_container->get_point_cloud(include_invalid_depth_data, __FUNCTION__);
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> MarkerDetectorContainer::get_point_cloud(
    std::shared_ptr<Eigen::Matrix3Xd> rotation,
    std::shared_ptr<Eigen::Vector3d> translation,
    bool include_invalid_depth_data, const char* caller)
{
    Logger::Instance()->Info("MarkerDetectorContainer.get_point_cloud called by " + std::string(caller));
    if(!this->cached_container)
    {
      this->setup_cached_container();
    }
    return this->cached_container->get_point_cloud(rotation, translation, include_invalid_depth_data, __FUNCTION__);
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> MarkerDetectorContainer::get_point_cloud(
    std::shared_ptr<Eigen::Affine3d> transform,
    bool include_invalid_depth_data, const char* caller)
{
  Logger::Instance()->Info("MarkerDetectorContainer.get_point_cloud called by " + std::string(caller));
  if(!this->cached_container)
  {
    this->setup_cached_container();
  }
  return this->cached_container->get_point_cloud(transform, include_invalid_depth_data, __FUNCTION__);
}

void MarkerDetectorContainer::refresh(bool align_frames, bool validate_frames, bool num_retries)
{
    this->session->refresh();
    this->cached_container = nullptr;
}

bool MarkerDetectorContainer::set_emitter(bool state)
{
  return this->session->set_emitter(state);
}

std::shared_ptr<cv::Mat> MarkerDetectorContainer::get_color_mat()
{
  return this->session->get_color_mat();
}

std::shared_ptr<cv::Mat> MarkerDetectorContainer::get_depth_mat(bool aligned_frames)
{
  return this->session->get_depth_mat();
}

std::shared_ptr<rs2_intrinsics> MarkerDetectorContainer::get_color_stream_intrinsics()
{
  return this->session->get_color_stream_intrinsics();
}

std::shared_ptr<rs2_intrinsics> MarkerDetectorContainer::get_depth_stream_intrinsics()
{
  return this->session->get_depth_stream_intrinsics();
}

std::shared_ptr<rs2_extrinsics> MarkerDetectorContainer::get_color_to_depth_extrinsics()
{
  return this->session->get_color_to_depth_extrinsics();
}

std::shared_ptr<rs2_extrinsics> MarkerDetectorContainer::get_depth_to_color_extrinsics()
{
  return this->session->get_depth_to_color_extrinsics();
}

std::shared_ptr<Eigen::Affine3d> MarkerDetectorContainer::get_transform()
{
  if(!this->cached_container)
  {
    this->setup_cached_container();
  }
  return this->cached_container->get_transform();
}

float MarkerDetectorContainer::depth_at_pixel(int x, int y)
{
  if(!this->cached_container)
  {
    this->setup_cached_container();
  }
  return this->cached_container->depth_at_pixel(x,y);
}

float MarkerDetectorContainer::camera_depth_at_pixel(int x, int y)
{
  float result = this->session->depth_at_pixel(x,y);
  Logger::Instance()->Trace("camera_depth_at_pixel method in MarkerDetectorContainer returns with result: ({},{},{})", x, y, result);
  return result;
}
