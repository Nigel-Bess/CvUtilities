//
// Created by steve on 5/18/20.
//

#include <Fulfil.Dispense/drop.h>
#include <Fulfil.DepthCam/core.h>
#include <Fulfil.DepthCam/aruco.h>
#include <Fulfil.DepthCam/mocks.h>
#include <Fulfil.DepthCam/point_cloud.h>
#include <eigen3/Eigen/Geometry>
#include <memory>
#include <Fulfil.DepthCam/visualization.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <ctime>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/file_system_util.h>

using fulfil::utils::FileSystemUtil;
using fulfil::utils::Logger;
using fulfil::depthcam::visualization::SessionVisualizer;
using fulfil::depthcam::pointcloud::LocalPointCloud;
using fulfil::depthcam::pointcloud::PixelPointCloud;
using fulfil::depthcam::pointcloud::PointCloud;
using std::cout;
using std::endl;
using std::shared_ptr;
using std::make_shared;

int calibration_routine_pixel_x;
int calibration_routine_pixel_y;

void onMouse_calibrate(int event, int x, int y, int flags, void* param)
{
    if (event == cv::EVENT_LBUTTONDBLCLK)
    {
        std::cout << "clicked on pixel   x= " << x << " y= " << y << std::endl;
        calibration_routine_pixel_x = x;
        calibration_routine_pixel_y = y;
    }
}

enum calibration_type {hover, dispense};

bool attempt_copy(const std::string& ini_write_path, std::shared_ptr<INIReader> previous_reader, std::shared_ptr<std::string> serial_number, calibration_type position)
{
    std::cout << "Attempting Copy of Former Calibration at position:" << position << "  (1 = dispense, 0 = hover)" << std::endl;

    std::vector<std::string> sections;

    if(position == dispense)
    {
      sections = {*serial_number + "_pixel_locations",
                                           *serial_number + "_camera_coordinates",
                                           *serial_number + "_tray_coordinates"};
    }
    else if (position == hover)
    {
      sections = {*serial_number + "_hover_pixel_locations",
                                           *serial_number + "_hover_camera_coordinates",
                                           *serial_number + "_hover_tray_coordinates"};
    }
    else
    {
      std::cout << "ERROR, ENUM TYPE DOES NOT EXIST" << std::endl;
    }

    std::vector<std::string> value_names = {"x", "y", "depth"};
    bool camera_present = true;
    std::for_each(sections.cbegin(), sections.cend(),
                  [&previous_reader, &camera_present, &value_names] (auto s){std::for_each(value_names.cbegin(), value_names.cend(),
                                                                                           [&previous_reader, &camera_present, &s](auto v){ camera_present &= previous_reader->checkForValue(s, v);});});

    if (!camera_present) {
        std::cout << "Camera number " << *serial_number << " was missing sections in previous ini. Moving on without copying." << std::endl;
        return camera_present;
    }
    for (const auto s : sections)
    {
        if (previous_reader->copySectionValuesToIniFile(ini_write_path,
                                                        s, value_names,true, true) < 0)
        {
            std::cout << "Issue getting valid section for " << s << " in previous ini. Moving on without copying section." << std::endl;
            camera_present = false;
        }
    }
    std::cout << "Camera number " << *serial_number << " copy complete!";
    if (!camera_present)
        std::cout << " But there were issues during copy. There may be inconsistencies in the data.";
    std::cout<< std::endl;
    return camera_present;
}


// returns index vector in desired order (Top Left, Top Right, Bottom Right, Bottom Left)
std::vector<int> sort_idx_clockwise(const std::vector<std::pair <int, int>>& pixel_vec)
{
    std::vector<int> idx(pixel_vec.size());
    std::iota(idx.begin(), idx.end(), 0);
    auto ycomp = [&pixel_vec](int idx1, int idx2) { return pixel_vec[idx1].second < pixel_vec[idx2].second; };
    std::stable_sort(idx.begin(), idx.end(), ycomp);
    auto xcomp = [&pixel_vec, &idx](int idx1, int idx2) { return pixel_vec[idx[idx1]].first < pixel_vec[idx[idx2]].first; };
    if (!xcomp(0, 1)) std::swap(idx[0], idx[1]);
    if (!xcomp(3, 2)) std::swap(idx[3], idx[2]);

    return idx;
}

bool validate_calibration_points(std::vector<std::pair <int, int>>& calibration_xy, std::vector<std::shared_ptr<Eigen::Matrix3Xd>>& camera_points)
{
    //TODO if there are any additional checks for validity put here
    if (calibration_xy.size() != camera_points.size() && calibration_xy.size() != 4)
        return false;
    std::vector<int> point_order = sort_idx_clockwise(calibration_xy);
    std::for_each(point_order.begin(), point_order.end(), [](int p) {std::cout << " idx: " << p;});
    std::cout << std::endl;
    for (int i = 0; i < 4; i++)
    {
        if (point_order[i] != i)
        {
            std::swap(calibration_xy[i], calibration_xy[point_order[i]]);
            std::swap(camera_points[i], camera_points[point_order[i]]);
            std::swap(point_order[i], point_order[point_order[i]]);
        }
    }
    return true;
}

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
    for (int i = 0; i < 4; i++) {std::cout << " value: (" << x[i] << ", " << y[i] << ", " << depth[i] << ") ";}
    std::cout << std::endl;
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
    for (int i = 0; i < 4; i++) {std::cout << " value: (" << x[i] << ", " << y[i] << ", " << depth[i] << ") ";}
    std::cout << std::endl;
    add_dims(ini_write_path, x,y,depth);
}

void format_add_dims(const std::string& ini_write_path){
    std::string x = "-0.336 0.336 0.336 -0.336";
    std::string y =  "-0.342 -0.342 0.342 0.342";
    std::string d = "0 0 0 0";
    add_dims(ini_write_path, x, y, d);
}

void write_calibration_to_ini(const std::string& ini_write_path,
                              const std::shared_ptr<std::string> serial_number,
                              std::vector<std::pair <int, int>>& calibration_xy,
                              std::vector<std::shared_ptr<Eigen::Matrix3Xd>>& camera_points,
                              calibration_type tray_position)
{
  if(tray_position == dispense)
  {
    INIReader::addSectionToIniFile(ini_write_path, *serial_number + std::string("_pixel_locations"));
    format_add_dims(ini_write_path, calibration_xy);
    INIReader::addSectionToIniFile(ini_write_path, *serial_number + std::string("_camera_coordinates"));
    format_add_dims(ini_write_path, camera_points);
    INIReader::addSectionToIniFile(ini_write_path, *serial_number + std::string("_tray_coordinates"));
    format_add_dims(ini_write_path);
  }
  else if(tray_position == hover)
  {
    INIReader::addSectionToIniFile(ini_write_path, *serial_number + std::string("_hover_pixel_locations"));
    format_add_dims(ini_write_path, calibration_xy);
    INIReader::addSectionToIniFile(ini_write_path, *serial_number + std::string("_hover_camera_coordinates"));
    format_add_dims(ini_write_path, camera_points);
    INIReader::addSectionToIniFile(ini_write_path, *serial_number + std::string("_hover_tray_coordinates"));
    format_add_dims(ini_write_path);
  }
}

std::shared_ptr<Eigen::Matrix3Xd> select_calibration_point(std::shared_ptr<fulfil::depthcam::Session> session, shared_ptr<cv::Mat> RGB_matrix,
                                                           std::shared_ptr<std::string> window_name, std::shared_ptr<std::pair <int, int>>  window_size,
                                                           std::shared_ptr<std::pair <int, int>> location_top_left, std::shared_ptr<std::pair <int, int>> location_top_right)
{
    float depth = 0;
    for (int i = 4; i >= 0; i--)
    {
        std::cout << "Double click on RGB image to find a pixel location, then click Enter to check depth value" << std::endl;

        cv::namedWindow(window_name->c_str(), cv::WINDOW_NORMAL);
        cv::setMouseCallback(window_name->c_str(), onMouse_calibrate, 0);     //allows for printing out pixel coordinate when image is double clicked
        cv::imshow(window_name->c_str(), *RGB_matrix);
        cv::resizeWindow(window_name->c_str(), window_size->first,window_size->second );
        cv::moveWindow(window_name->c_str(), location_top_left->first, location_top_left->second);
        cv::waitKey(0);

        depth = session->depth_at_pixel(round(calibration_routine_pixel_x), round(calibration_routine_pixel_y));  //get depth data at input pixel (x,y) in camera coordinates, using realsense library function
        std::cout << "Depth of selected pixel is " << depth << ". ";
        if (depth == 0)
            std::cout << "Issue getting depth at pixel. Depth must be non-zero. " << i << " out of 5 tries left."<<std::endl;
        else
            break;
    }
    cv::destroyWindow (window_name->c_str());
    if (depth == 0) {
        std::string err_msg = "Repeated user error! Invalid pixel depths repeatedly selected!";
        std::cout << err_msg << std::endl;
        throw std::runtime_error(err_msg);
    }
    std::cout << std::endl;
    std::shared_ptr<std::vector<std::shared_ptr<std::pair<std::shared_ptr<cv::Point2f>, float>>>> point =
            make_shared<std::vector<std::shared_ptr<std::pair<std::shared_ptr<cv::Point2f>, float>>>>();
    point->push_back(make_shared<std::pair<shared_ptr<cv::Point2f>,float>>(make_shared<cv::Point2f>(calibration_routine_pixel_x, calibration_routine_pixel_y), depth));
    std::shared_ptr<PointCloud> cloud = session->get_point_cloud(true)->as_pixel_cloud()->new_point_cloud(point);
    std::shared_ptr<Eigen::Matrix3Xd> data = cloud->as_camera_cloud()->get_data();

    //Return the new point
    return data;

}

bool get_calibration(std::shared_ptr<fulfil::depthcam::Session> session,
                     std::vector<std::shared_ptr<Eigen::Matrix3Xd>>& camera_points,
                     std::vector<std::pair <int, int>>& calibration_xy)
{

    // Create the session visualizer
    std::shared_ptr<std::string> window_name_1 = std::make_shared<std::string>("Visualizer of RGB");
    std::shared_ptr<std::string> window_name_0 = std::make_shared<std::string>("Visualizer of Depth");

    std::shared_ptr<std::pair <int, int>> location_top_left = std::make_shared<std::pair <int, int>>(0,0);
    std::shared_ptr<std::pair <int, int>> location_top_right = std::make_shared<std::pair <int, int>>(1000, 0);
    std::shared_ptr<std::pair <int, int>> window_size = std::make_shared<std::pair <int, int>>(960,540); // 960, 540   //1280,  720 for one monitor,  960, 540  for laptop


    std::vector<std::string> point_prompts = {"Top Left", "Top Right", "Bottom Right", "Bottom Left"};

    for (const auto pp : point_prompts) {
        std::cout << "Please select the " << pp << " calibration point in the image." << std::endl;
        session->refresh();
        shared_ptr<cv::Mat> RGB_matrix = session->get_color_mat();

        shared_ptr<SessionVisualizer> session_visualizer = make_shared<SessionVisualizer>(session, window_name_0, location_top_right, window_size, 0);
        std::shared_ptr<PointCloud> point_cloud = session->get_point_cloud(false)->as_pixel_cloud();
        session_visualizer->display_image(session_visualizer->display_points_with_depth_coloring(point_cloud));


        std::shared_ptr<Eigen::Matrix3Xd> data = select_calibration_point(session, RGB_matrix, window_name_1, window_size, location_top_left, location_top_right);

        std::cout << "Calibration pixel: ("<< calibration_routine_pixel_x << ", " << calibration_routine_pixel_y <<
                  ")  Camera point value: (" << (*data)(0,0) <<  ", "  <<  (*data)(1,0) << ", " <<  (*data)(2,0) << ")" << std::endl;
        calibration_xy.push_back(std::pair <int, int>(calibration_routine_pixel_x, calibration_routine_pixel_y));
        camera_points.push_back(data);
    }

    return validate_calibration_points(calibration_xy, camera_points);

}

bool try_camera_calibration_cycle(std::shared_ptr<fulfil::depthcam::Session> session, std::shared_ptr<INIReader> previous_reader, const std::string& ini_write_path){
    std::shared_ptr<std::string> serial_number = session->get_serial_number();
    std::cout << "We will begin calibrating camera serial no. " << *serial_number << std::endl;

    calibration_type tray_position; // hover or dispense
    char hover_dispense = '\0';
    auto get_action = [&hover_dispense]() { if (hover_dispense == 'h') return 1; if (hover_dispense == 'd') return 0; return -1; };

    std::string err_msg = "Whoopsie! valid inputs include lowercase h and lowercase d only. Let's try again.";

    while(true)
    {
      cout << "Is the tray at hover position (h) or dispense position (d)" << "?\nType [h/d]" << std::endl;
      try {
        std::cin >> hover_dispense ;
      } catch(...) {
        hover_dispense  = '\0';
        std::cout << err_msg << std::endl;
      }
      if (get_action() < 0) std::cout << err_msg << std::endl;
      else break;
    }
    if (get_action() == 1) tray_position = hover;
    if (get_action() == 0) tray_position = dispense;


    std::vector<std::shared_ptr<Eigen::Matrix3Xd>> camera_points;
    std::vector<std::pair <int, int>> calibration_xy;
    bool success;
    try {
        success = get_calibration(session, camera_points, calibration_xy);
    } catch (const std::exception& ex) {
      std::cout << "Caught exception in calibration cycle:\n" << ex.what() << std::endl;
        success = false;
    }
    if (success) {
        if(tray_position == dispense)
        {
          write_calibration_to_ini(ini_write_path, serial_number, calibration_xy, camera_points, tray_position);
          attempt_copy(ini_write_path, previous_reader, serial_number, hover);
        }
        else
        {
          attempt_copy(ini_write_path, previous_reader, serial_number, dispense);
          write_calibration_to_ini(ini_write_path, serial_number, calibration_xy, camera_points, tray_position);
        }
        return 1;
    }

    std::cout << "Issue getting calibration, skipping file update for camera "
              << *serial_number << " . Copying over previous calibrations for now." << std::endl;

    bool successful_copy = (attempt_copy(ini_write_path, previous_reader, serial_number, dispense) and
        attempt_copy(ini_write_path, previous_reader, serial_number, hover));

    return successful_copy;
}

int main(int argc, char** argv)
{

    Logger::Instance()->SetConsoleLogLevel(Logger::Level::TurnOff);
    Logger::Instance()->SetFileLogLevel(Logger::Level::TurnOff);

    if(argc < 2)
    {
        std::cout << "No argument means calibration will be performed on online cameras" << std::endl;

        // Getting a session
        std::shared_ptr<fulfil::depthcam::DeviceManager> manager = std::make_shared<fulfil::depthcam::DeviceManager>();
        std::shared_ptr<std::vector<std::shared_ptr<fulfil::depthcam::Session>>> connected_sessions =
          manager->get_connected_sessions();
        std::cout << connected_sessions->size() << " sensor(s) were detected on this device" << std::endl << std::endl;

        char hostname[255];
        memset(hostname, 0, sizeof(hostname));
        gethostname(hostname, sizeof(hostname));

        std::string ini_write_path = INIReader::get_compiled_default_dir_prefix();
        FileSystemUtil::join_append(ini_write_path, "tray_calibration_data.ini");
        std::shared_ptr<INIReader> previous_reader = std::make_shared<INIReader>(ini_write_path, false);
        INIReader::addSectionToIniFile(ini_write_path, "config_details", false);
        INIReader::addValueToIniFile(ini_write_path,"hostname", std::string(hostname));
        time_t now = std::time(0);
        INIReader::addValueToIniFile(ini_write_path,"date_updated", now);

        char yes_no = '\0';
        auto get_action = [&yes_no]() { if (yes_no == 'y') return 1; if (yes_no == 'n') return 0; return -1; };

        std::string err_msg = "Whoopsie! valid inputs include lowercase y and lowercase n only. Let's try again.";
        for (int i = 0; i < connected_sessions->size(); i++ ){
            //"Not enough points selected, error in calibration process. Please restart!"
            std::shared_ptr<fulfil::depthcam::Session> session = connected_sessions->at(i);
            std::shared_ptr<std::string> serial_number = session->get_serial_number();
            while(true) {
                cout << "Would you like to calibrate camera #"  << i
                     <<", serial number " << *serial_number <<"?\nType [y/n]" << std::endl;
                try {
                    std::cin >> yes_no;
                } catch(...) {
                    yes_no = '\0';
                    std::cout << err_msg << std::endl;
                }
                if (get_action() < 0) std::cout << err_msg << std::endl;
                else break;
            }
            if (get_action())
            {
                try_camera_calibration_cycle(session, previous_reader, ini_write_path);
            } else {
                std::cout << "You have chosen to skip file update for camera "
                          << *serial_number << " for now." << std::endl;

                attempt_copy(ini_write_path, previous_reader, serial_number, dispense);
                attempt_copy(ini_write_path, previous_reader, serial_number, hover);
            }
        }

  }
  else  // get mock session from saved data
  {
    std::vector<std::shared_ptr<Eigen::Matrix3Xd>> camera_points;
    std::vector<std::pair <int, int>> calibration_xy;
    std::cout << "The directory path passed in was: " << argv[1] << std::endl;
    std::shared_ptr<std::string> directory_path = std::make_shared<std::string>(argv[1]);
    std::string ini_write_path = INIReader::get_compiled_default_dir_prefix();
    FileSystemUtil::join_append(ini_write_path, "tray_mock_calibration_data.ini");
    std::ifstream backup_stream(ini_write_path);
    bool backup_exists = backup_stream.good();
    std::filebuf* backup_buf = backup_stream.rdbuf();

    // Back up mock file
    std::size_t size = backup_buf->pubseekoff (0,backup_stream.end,backup_stream.in);
    backup_buf->pubseekpos (0,backup_stream.in);
    char* backup_data;
    if (backup_exists) {
        backup_data=new char[size];
        backup_buf->sgetn (backup_data, size);
    }
    backup_stream.close();

    std::shared_ptr<INIReader> previous_reader = std::make_shared<INIReader>(ini_write_path, false);
    std::string mock_serial = previous_reader->Get("config_details", "mock_serial", "111111111111");
    std::shared_ptr<fulfil::depthcam::mocks::MockSession> mock_session = std::make_shared<fulfil::depthcam::mocks::MockSession>(directory_path, mock_serial);
    INIReader::addSectionToIniFile(ini_write_path, "config_details", false);
    time_t now = std::time(0);
    INIReader::addValueToIniFile(ini_write_path,"date_updated", now);
    INIReader::addValueToIniFile(ini_write_path,"mock_data_directory_path", *directory_path);
    INIReader::addValueToIniFile(ini_write_path,"mock_serial", mock_serial);

    if (!try_camera_calibration_cycle(mock_session, previous_reader, ini_write_path) && backup_exists)
    {
        std::ofstream fixit(ini_write_path, std::ios::trunc);
        fixit << backup_data;
        fixit.close();
        delete[] backup_data;
        std::cout << "Calibration cycle failed. Rolling back all changes to mock ini file." << std::endl;
    }
  }
    cv::destroyAllWindows();
    return 0;
}
