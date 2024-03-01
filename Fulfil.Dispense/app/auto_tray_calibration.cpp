#include <Fulfil.Dispense/drop.h>
#include <Fulfil.DepthCam/core.h>
#include <Fulfil.DepthCam/point_cloud.h>
#include"Fulfil.DepthCam/aruco/marker.h"
#include <eigen3/Eigen/Geometry>
#include <memory>
#include <Fulfil.DepthCam/visualization.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <ctime>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/file_system_util.h>
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include <Fulfil.DepthCam/aruco/marker_detector.h>
#include <FulfilMongoCpp/mongo_connection.h>
#include <FulfilMongoCpp/mongo_filter/mongo_filters.h>
#include <Fulfil.Dispense/mongo/mongo_tray_calibration.h>

using ff_mongo_cpp::mongo_filter::MongoFilter;
using ff_mongo_cpp::MongoConnection;
using ff_mongo_cpp::mongo_objects::MongoObjectID;
using fulfil::depthcam::aruco::MarkerDetector;
using fulfil::utils::FileSystemUtil;
using fulfil::utils::Logger;
using fulfil::depthcam::visualization::SessionVisualizer;
using fulfil::depthcam::pointcloud::LocalPointCloud;
using fulfil::depthcam::pointcloud::PixelPointCloud;
using fulfil::depthcam::pointcloud::PointCloud;
using fulfil::depthcam::aruco::Marker;
using fulfil::mongo::MongoTrayCalibration;


enum calibration_type {hover, dispense, tongue, induction};
//                      0         1        2        3         4         5         6         7         8         9         10        11
float markers_x[12] = {-0.40225, -0.23497, 0.18750, 0.40225, -0.43375,  0.43375, -0.43375,  0.43375, -0.40225, -0.23442,  0.26248,  0.40225};
float markers_y[12] = {-0.40025, -0.40025,-0.40025,-0.40025, -0.28800, -0.20865,  0.10800,  0.19619,  0.40025,  0.40025,  0.40025,  0.40025};
float markers_z[12] = {-0.002,   -0.012,  -0.022,  -0.002,   -0.002,   -0.022,   -0.012,   -0.002,   -0.002,   -0.012,   -0.022,   -0.002};

std::string ini_base_path;
int min_markers_required;


// JFC clean this shit up wtf

template<typename T>
void add_dims(const std::string& ini_write_path, T&& x, T&&  y, T&& depth){
    INIReader::addValueToIniFile(ini_write_path,"x", std::forward<T>(x));
    INIReader::addValueToIniFile(ini_write_path,"y", std::forward<T>(y));
    INIReader::addValueToIniFile(ini_write_path,"depth", std::forward<T>(depth));
}

void format_add_dims(const std::string& ini_write_path, const std::vector<std::pair <int, int>>& points){
    std::vector<int> x;
    std::vector<int> y;
    std::vector<int> depth;
    auto unzip_point = [&x, &y, &depth](std::pair <int, int> pixel) {x.push_back(pixel.first); y.push_back(pixel.second); depth.push_back(0); };
    std::for_each( points.begin(), points.end(), unzip_point);
    add_dims(ini_write_path, x,y,depth);
}

void format_add_dims(const std::string& ini_write_path,
                     const std::vector<std::shared_ptr<Eigen::Matrix3Xd>>& points){
    std::vector<double> x;
    std::vector<double> y;
    std::vector<double> depth;
    auto unzip_point = [&x, &y, &depth](std::shared_ptr<Eigen::Matrix3Xd> point) {
        x.push_back(((*point)(0,0)));
        y.push_back(((*point)(1,0)));
        depth.push_back(((*point)(2,0))); };
    std::for_each( points.begin(), points.end(), unzip_point);
    add_dims(ini_write_path, x,y,depth);
}

//TODO add new tray points here
void format_add_dims(const std::string& ini_write_path,
                     const std::vector<int>& marker_ids){
  std::string x = "";
  std::string y = "";
  std::string d = "";
  for (int i = 0; i < (int)marker_ids.size(); i++){
      x = x + std::to_string(markers_x[marker_ids.at(i)]) + " ";
      y = y + std::to_string(markers_y[marker_ids.at(i)]) + " ";
      d = d + std::to_string(markers_z[marker_ids.at(i)]) + " ";
  }
  add_dims(ini_write_path, x, y, d);
}

void wipe_file(const std::string& ini_write_path){
    std::ofstream del;
    del.open(ini_write_path, std::ofstream::out | std::ofstream::trunc);
    del.close();
}

void write_calibration_to_ini(const std::string& ini_write_path,
                              const std::string &serial_number,
                              std::vector<std::pair <int, int>>& calibration_xy,
                              std::vector<std::shared_ptr<Eigen::Matrix3Xd>>& camera_points,
                              std::vector<int>& marker_ids,
                              calibration_type tray_position)
{
  if(tray_position == dispense)
  {
    INIReader::addSectionToIniFile(ini_write_path, serial_number + std::string("_pixel_locations"));
    format_add_dims(ini_write_path, calibration_xy);
    INIReader::addSectionToIniFile(ini_write_path, serial_number + std::string("_camera_coordinates"));
    format_add_dims(ini_write_path, camera_points);
    INIReader::addSectionToIniFile(ini_write_path, serial_number + std::string("_tray_coordinates"));
    format_add_dims(ini_write_path, marker_ids);
  }
  else if(tray_position == hover)
  {
    INIReader::addSectionToIniFile(ini_write_path, serial_number + std::string("_hover_pixel_locations"));
    format_add_dims(ini_write_path, calibration_xy);
    INIReader::addSectionToIniFile(ini_write_path, serial_number + std::string("_hover_camera_coordinates"));
    format_add_dims(ini_write_path, camera_points);
    INIReader::addSectionToIniFile(ini_write_path, serial_number + std::string("_hover_tray_coordinates"));
    format_add_dims(ini_write_path, marker_ids);
  }
  else if(tray_position == tongue)
  {
      INIReader::addSectionToIniFile(ini_write_path, serial_number + std::string("_tongue_pixel_locations"));
      format_add_dims(ini_write_path, calibration_xy);
      INIReader::addSectionToIniFile(ini_write_path, serial_number + std::string("_tongue_camera_coordinates"));
      format_add_dims(ini_write_path, camera_points);
      INIReader::addSectionToIniFile(ini_write_path, serial_number + std::string("_tongue_tray_coordinates"));
      format_add_dims(ini_write_path, marker_ids);
  }
  else if(tray_position == induction)
  {
      INIReader::addSectionToIniFile(ini_write_path, serial_number + std::string("_induction_pixel_locations"));
      format_add_dims(ini_write_path, calibration_xy);
      INIReader::addSectionToIniFile(ini_write_path, serial_number + std::string("_induction_camera_coordinates"));
      format_add_dims(ini_write_path, camera_points);
      INIReader::addSectionToIniFile(ini_write_path, serial_number + std::string("_induction_tray_coordinates"));
      format_add_dims(ini_write_path, marker_ids);
  }
}

MongoObjectID upload_calibration_to_mongo(const std::shared_ptr<std::string> serial_number,
                                 std::vector<std::pair <int, int>>& calibration_xy,
                                 std::vector<std::shared_ptr<Eigen::Matrix3Xd>>& camera_points,
                                 std::vector<int>&marker_ids,
                                 std::string position){

    //use mongo config file to connect to mongo
    std::shared_ptr<INIReader> mongo_reader = std::make_shared<INIReader>("secret/mongo_conn_config.ini", true);
    if (mongo_reader->ParseError() < 0) {
        Logger::Instance()->Fatal("Failure to load mongo credentials, check path.");
        throw std::runtime_error("Failure to parse ini file...");
    }
    std::string conn_str = mongo_reader->Get("connection_info", "conn_string");
    std::shared_ptr<MongoConnection> mongo_conn = std::make_shared<MongoConnection>(conn_str);
    Logger::Instance()->Info("Able to make connection to factory mongo: {}", mongo_conn->IsConnected());
    if (!mongo_conn->IsConnected()){
        Logger::Instance()->Fatal("Failure to make connection to MongoDB. Exiting program with fail!");
    }

    MongoObjectID inserted_oid;

    std::vector<float> tray_x;
    std::vector<float> tray_y;
    std::vector<float> tray_z;
    for (int i = 0; i < (int)marker_ids.size(); i++){
        tray_x.push_back(markers_x[marker_ids.at(i)]);
        tray_y.push_back(markers_y[marker_ids.at(i)]);
        tray_z.push_back(markers_z[marker_ids.at(i)]);
    }

    //get name of machine running calibration
    char hostname[255];
    memset(hostname, 0, sizeof(hostname));
    gethostname(hostname, sizeof(hostname));
    std::string machine(hostname);
    std::shared_ptr<MongoTrayCalibration> doc = std::make_shared<MongoTrayCalibration>(machine, *serial_number,calibration_xy,
                                                                                       camera_points, tray_x, tray_y, tray_z, position);
    //upload json to mongo connection
    mongo_conn->tryInsertOne(doc, inserted_oid);
    //add tray calibration oid to Machines.DepthCameras so main can reference it
    nlohmann::json update;
    update["last_tray_camera_calibration_"+position]["$oid"] = inserted_oid.str();
    MongoFilter update_filter = MongoFilter("tray_camera_serial_number", MongoFilter::FilterType::Eq, *serial_number);
    if (!mongo_conn->tryUpdateOneByFilter(update_filter, std::make_shared<ff_mongo_cpp::mongo_objects::MongoJsonDocument>(update, "DepthCameras", "Machines"), "$set")){
        Logger::Instance()->Error("Failed to modify any Machines.DepthCameras documents, check a document in collection has this camera's serial number.");
        exit(1);
    }
    return inserted_oid;
}

//validate markers function adapted from marker_Detector.cpp method
//TODO add marker x,y coordinate check, currently only depth checks
bool validate_markers(std::shared_ptr<fulfil::depthcam::Session> session,
                      std::shared_ptr<std::vector<std::shared_ptr<Marker>>> markers,
                      std::vector<std::shared_ptr<Eigen::Matrix3Xd>>& camera_points,
                      std::vector<std::pair <int, int>>& calibration_xy,
                      std::vector<int>& marker_ids)
{
  //check that markers are within search zone and at expected depth
  int num_detected_markers = markers->size();

  if(num_detected_markers < min_markers_required)
  {
    Logger::Instance()->Error("Did not detect at least {} markers as expected, instead only detected {} markers with IDs: ", min_markers_required, num_detected_markers);
    for(int i = 0; i < num_detected_markers; i++)
    {
      std::shared_ptr<Marker> marker = markers->at(i);
      std::cout << marker->get_id() << std::endl;
    }
    return false;
  }

  float depth_min = 0.5;
  float depth_max = 1.5;

  for(int i = 0; i < num_detected_markers; i++)
  {
    std::shared_ptr<Marker> marker = markers->at(i);
    int marker_id = marker->get_id();
    float marker_x = marker->get_coordinate(fulfil::depthcam::aruco::Marker::center)->x;
    float marker_y = marker->get_coordinate(fulfil::depthcam::aruco::Marker::center)->y;

    //validate marker is within acceptable region of RGB image. Calls depth session function to use aligned depth frame
    float detected_depth = session->depth_at_pixel(round(marker_x), round(marker_y));
    Logger::Instance()->Debug("Validating location and depth of marker: {}", marker_id);

    if(detected_depth < depth_min or detected_depth > depth_max)
    {
      Logger::Instance()->Warn("Marker ID {} at ({},{})  had detected depth outside of acceptable range and was removed. Detected: {}, min acceptable: {}, max acceptable: {}",
                               marker->get_id(), marker_x, marker_y, detected_depth, depth_min, depth_max);
      return false;
    }
    //get camera coordinates, this is required for the calibration file
    std::shared_ptr<std::vector<std::shared_ptr<std::pair<std::shared_ptr<cv::Point2f>, float>>>> point =
    std::make_shared<std::vector<std::shared_ptr<std::pair<std::shared_ptr<cv::Point2f>, float>>>>();
    point->push_back(std::make_shared<std::pair<std::shared_ptr<cv::Point2f>,float>>(std::make_shared<cv::Point2f>(marker_x, marker_y), detected_depth));
    std::shared_ptr<PointCloud> cloud = session->get_point_cloud(true)->as_pixel_cloud()->new_point_cloud(point);
    std::shared_ptr<Eigen::Matrix3Xd> data = cloud->as_camera_cloud()->get_data();

    calibration_xy.push_back(std::pair <int, int>(marker_x, marker_y));
    camera_points.push_back(data);
    marker_ids.push_back(markers->at(i)->get_id());
  }
  return true;
}

bool get_calibration(std::shared_ptr<fulfil::depthcam::Session> session,
                     std::vector<std::shared_ptr<Eigen::Matrix3Xd>>& camera_points,
                     std::vector<std::pair <int, int>>& calibration_xy,
                     std::vector<int>& marker_ids,
                     std::string position)
{
    session->refresh();

    std::shared_ptr<cv::Mat> RGB_image = session->get_color_mat();
    cv::Mat image_to_save;
    RGB_image->copyTo(image_to_save);

    //TODO: adjust path based on hover vs. dispense calibration
    std::string write_path = ini_base_path + "/" + position + "_calibration_image.jpg";
    cv::imwrite(write_path, image_to_save);
    MarkerDetector detector = MarkerDetector(12, 4);

    Logger::Instance()->Info("Captured and wrote image to local device. Running marker detector now.");

    // 5 attempts at finding and validating markers
    int num_attempts = 10;
    for(int i = 0; i < num_attempts; i++)
    {
      Logger::Instance()->Info("Marker Detector attempt: {}", i);
      std::shared_ptr<std::vector<std::shared_ptr<Marker>>> detected_markers = detector.detect_markers(RGB_image);
      bool validation_check = validate_markers(session,detected_markers, camera_points, calibration_xy, marker_ids); //validates depth and within search region
      if(validation_check) break; //break out of loop if all markers were detected and valid
      if(i == num_attempts - 1) return false;
      session->refresh(); //take a new image to try again
      RGB_image = session->get_color_mat();
    }

    Logger::Instance()->Info("Markers all found to be valid, and camera coordinates for calibration have been found.");
    std::vector<int> id_vec;
    return true;
}

bool try_camera_calibration_cycle(std::shared_ptr<fulfil::depthcam::Session> session,
                                  const std::string& ini_write_path, calibration_type tray_position){
    std::string serial_number = *session->get_serial_number();
    std::cout << "We will begin calibrating camera serial no. " << serial_number << std::endl;

    std::vector<std::shared_ptr<Eigen::Matrix3Xd>> camera_points;
    std::vector<std::pair <int, int>> calibration_xy;
    std::vector<int> marker_ids;
    bool success;

    std::string position;
    switch (tray_position) {
        case dispense:
            position = "dispense";
            break;
        case hover:
            position = "hover";
            break;
        case tongue:
            position = "tongue";
            break;
        case induction:
            position = "induction";
            break;
    }

    try
    {
        success = get_calibration(session, camera_points, calibration_xy, marker_ids, position);
    }
    catch (const std::exception& ex)
    {
      Logger::Instance()->Info("Issue detected during calibration. Will not write to file.");
      success = false;
    }
    if (success)
    {
        Logger::Instance()->Info("Writing new calibration position to file now.");

        char hostname[255];
        memset(hostname, 0, sizeof(hostname));
        gethostname(hostname, sizeof(hostname));

        INIReader::addSectionToIniFile(ini_write_path, "config_details", false);
        INIReader::addValueToIniFile(ini_write_path,"hostname", std::string(hostname));
        time_t now = std::time(0);
        INIReader::addValueToIniFile(ini_write_path,"date_updated", now);

        if (tray_position != induction){
	    // MongoObjectID res{};
            MongoObjectID res = upload_calibration_to_mongo(session->get_serial_number(), calibration_xy, camera_points, marker_ids, position);
            if (res.is_null()){
                Logger::Instance()->Error("Failed to upload calibration results to Mongo");
                exit(1);
            }
            INIReader::addValueToIniFile(ini_write_path, serial_number+"_recipe_id_"+position, res.str());
        }

        write_calibration_to_ini(ini_write_path, serial_number, calibration_xy, camera_points, marker_ids, tray_position);
        Logger::Instance()->Info("Calibration was a success!!!");
        return 1;
    }

    Logger::Instance()->Error("Issue getting calibration. Nothing will be written to file.");
    return 0;
}

int main(int argc, char** argv)
{
  calibration_type tray_position;
  if(argc != 3)
  {
    std::cout << "Program takes as arguments Dispense, Hover, Tongue, or Induction to specify calibration type. "
                 "It also takes the min number of markers needed. Try again" << std::endl;
    return 1;
  }
  else if (strcmp(argv[1], "Dispense") == 0)
  {
    std::cout << "User indicated that calibration takes place at Dispense height" << std::endl;
    tray_position = dispense;
  }
  else if (strcmp(argv[1], "Hover") == 0)
  {
    std::cout << "User indicated that calibration takes place at Hover height" << std::endl;
    tray_position = hover;
  }
  else if (strcmp(argv[1], "Tongue") == 0)
  {
      std::cout << "User indicated that calibration takes place at Tongue Engage height" << std::endl;
      tray_position = tongue;
  }
  else if (strcmp(argv[1], "Induction") == 0)
  {
      std::cout << "User indicated that induction camera is being calibrated" << std::endl;
      tray_position = induction;
  }
  else
  {
    std::cout << "Argument must be Dispense, Hover, Tongue, or Induction. Try again" << std::endl;
    exit(1);
  }
  int num_markers_needed = atoi(argv[2]);
  if(num_markers_needed < 4 or num_markers_needed > 12)
  {
    std::cout << "Number of markers required must be between 4 and 12" << std::endl;
    return 1;
  }
  min_markers_required = num_markers_needed;

  Logger::Instance()->SetConsoleLogLevel(Logger::Level::Info);
  Logger::Instance()->SetFileLogLevel(Logger::Level::Debug);

  std::cout << "Calibration will be performed on tray camera" << std::endl;

  std::string ini_write_path = INIReader::get_compiled_default_dir_prefix();
  ini_base_path = ini_write_path;
  if (tray_position == dispense){
    FileSystemUtil::join_append(ini_write_path, "new_tray_calibration_data_dispense.ini");
  }
  else if (tray_position == hover){
    FileSystemUtil::join_append(ini_write_path, "new_tray_calibration_data_hover.ini");
  }
  else if (tray_position == tongue){
    FileSystemUtil::join_append(ini_write_path, "new_tray_calibration_data_tongue.ini");
  }
  else{
      FileSystemUtil::join_append(ini_write_path, "new_tray_calibration_data_induction.ini");
  }

  Logger::Instance()->Info("Wiping file: {}", ini_write_path);
  wipe_file(ini_write_path);

  // Getting the tray session
  std::shared_ptr<fulfil::depthcam::DeviceManager> manager = std::make_shared<fulfil::depthcam::DeviceManager>();

  std::shared_ptr<INIReader> reader = std::make_shared<INIReader>("AGX_specific_main.ini", true);

  std::string serial_number = reader->Get("device_specific", "tray_cam", "err");
  if (tray_position == induction){
      serial_number = reader->Get("device_specific", "induction_cam", "err");
  }
  std::shared_ptr<fulfil::depthcam::Session> session = manager->session_by_serial_number(serial_number);

  try_camera_calibration_cycle(session, ini_write_path, tray_position);

  cv::destroyAllWindows();
  return 0;
}
