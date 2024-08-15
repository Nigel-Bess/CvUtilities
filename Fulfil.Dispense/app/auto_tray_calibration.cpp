#include <Fulfil.DepthCam/point_cloud.h>
#include <Fulfil.DepthCam/aruco/marker.h>
#include <eigen3/Eigen/Geometry>
#include <memory>
#include <iostream>
#include <filesystem>
#include <algorithm>
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
using fulfil::depthcam::pointcloud::LocalPointCloud;
using fulfil::depthcam::pointcloud::PixelPointCloud;
using fulfil::depthcam::pointcloud::PointCloud;
using fulfil::depthcam::aruco::Marker;
using fulfil::mongo::MongoTrayCalibration;


enum calibration_type {hover, dispense};
//std::array<const char*, 2> CalibrationFileName{"new_tray_calibration_data_hover.ini", "new_tray_calibration_data_dispense.ini"};
std::array<const char*, 2> TrayPositionNames {"hover", "dispense"};
std::string ini_base_path{};
int min_markers_required{16};

std::string calibration_filename(std::string_view position) { return std::string("tray_calibration_data_").append(position) + ".ini" ; }

//                       0        1        2        3        4        5        6        7        8        9        10       11       12       13       14       15
float markers_x[16] = { -0.3180, -0.1060,  0.1060,  0.3180, -0.3180, -0.1060,  0.1060,  0.3180, -0.3180, -0.1060,  0.1060,  0.3180, -0.3180, -0.1060,  0.1060,  0.3180 };
float markers_y[16] = { -0.3180, -0.3180, -0.3180, -0.3180, -0.1230, -0.1230, -0.1230, -0.1230,  0.0720,  0.0720,  0.0720,  0.0720,  0.2670,  0.2670,  0.2670,  0.2670 };
float markers_z[16] = { -0.0150, -0.0150, -0.0150, -0.0150, -0.0150, -0.0150, -0.0150, -0.0150, -0.0150, -0.0150, -0.0150, -0.0150, -0.0150, -0.0150, -0.0150, -0.0150 };



std::string_view get_option_value( const std::vector<std::string_view>& args,
                                   const std::string_view& option_name, const std::string_view& defaultval="") {
    if (args.empty()) { return defaultval; }
    if (args.size() < 2) {
        throw std::runtime_error("No option value pair present!");
    };
    auto option = std::find(args.begin(), args.end(), option_name);
    auto value = std::next(option, 1);
    if (option != args.end() && value != args.end()) {
        return *value;
    }
    return defaultval;
}

bool has_option_flag(const std::vector<std::string_view>& args,
                     const std::string_view& option_name) {
    return std::find(args.begin(), args.end(), option_name) != args.end();

}

struct calibration_args {
    std::shared_ptr<fulfil::depthcam::Session> session{};
    std::filesystem::path output_dir {INIReader::get_compiled_default_dir_prefix()};
    size_t min_tags{12};
    calibration_type lift_height{hover};
    calibration_args()=default;
    void parse_args(int argc, char **argv);
};

void calibration_args::parse_args(int argc, char **argv) {
    const std::vector<std::string_view> args(argv + 1, argv + argc);
    // LIFT POSITION
    if (auto position = get_option_value(args, "-p") ; !position.empty()) {
        auto height = std::distance(TrayPositionNames.begin(),
                                    std::find(TrayPositionNames.begin(), TrayPositionNames.end(), position));
        if (height == TrayPositionNames.size()) {
            Logger::Instance()->Error("Entered invalid lift calibration position {}!\n  Valid Options for -p: [Hover|Dispense]", position);
            exit(1);
        }
        Logger::Instance()->Info("Position Code: {}", height);
        this->lift_height = static_cast<calibration_type>(height);
    }

    Logger::Instance()->Info("Height: {}", TrayPositionNames[lift_height]);


    // MIN TAGS
    if (auto tags = get_option_value(args, "-m") ; !tags.empty()) {
        min_tags = std::stoul(std::string(tags));
    }
    Logger::Instance()->Info("Min Tags: {}", min_tags);

    // OUTPUT DIRECTORY
    this->output_dir = get_option_value(args, "-o", INIReader::get_compiled_default_dir_prefix());
    Logger::Instance()->Info("Output Dir: {}", output_dir.string());


    // SESSION CAMERA SERIAL
    auto serial = get_option_value(args, "-s");
    if (serial.empty()) {
        std::shared_ptr<fulfil::depthcam::DeviceManager> manager = std::make_shared<fulfil::depthcam::DeviceManager>();
        this->session = manager->get_connected_sessions()->back();
        Logger::Instance()->Warn("No serial number defined in command line. Using default serial {}!", *this->session->get_serial_number());
    } else {
        std::string serialstr = std::string(serial);
        Logger::Instance()->Info("Attempting to get specified camera: {}", serialstr);
        std::shared_ptr<fulfil::depthcam::DeviceManager> manager = std::make_shared<fulfil::depthcam::DeviceManager>(std::vector{serialstr}, false);
        this->session = manager->session_by_serial_number(serialstr);
    }
    this->session->set_sensor_name("Calibration_DAB");
}

template<typename BasicJsonType>
void to_json(BasicJsonType &j, const calibration_args& params)
{
    j = { { "serial_num", *params.session->get_serial_number() }, { "position", TrayPositionNames[params.lift_height] }, {"min_tags", params.min_tags} };
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
                              const std::shared_ptr<std::string> serial_number,
                              std::vector<std::pair <int, int>>& calibration_xy,
                              std::vector<std::shared_ptr<Eigen::Matrix3Xd>>& camera_points,
                              std::vector<int>& marker_ids,
                              calibration_type tray_position)
{
    if(tray_position == dispense)
    {
        INIReader::addSectionToIniFile(ini_write_path, *serial_number + std::string("_pixel_locations"));
        format_add_dims(ini_write_path, calibration_xy);
        INIReader::addSectionToIniFile(ini_write_path, *serial_number + std::string("_camera_coordinates"));
        format_add_dims(ini_write_path, camera_points);
        INIReader::addSectionToIniFile(ini_write_path, *serial_number + std::string("_tray_coordinates"));
        format_add_dims(ini_write_path, marker_ids);
    }
    else if(tray_position == hover)
    {
        INIReader::addSectionToIniFile(ini_write_path, *serial_number + std::string("_hover_pixel_locations"));
        format_add_dims(ini_write_path, calibration_xy);
        INIReader::addSectionToIniFile(ini_write_path, *serial_number + std::string("_hover_camera_coordinates"));
        format_add_dims(ini_write_path, camera_points);
        INIReader::addSectionToIniFile(ini_write_path, *serial_number + std::string("_hover_tray_coordinates"));
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

    std::stringstream msgs;
    msgs << "Detected " << num_detected_markers << " markers with IDs: ";
    for (int i = 0; i < num_detected_markers; i++)
    {
        std::shared_ptr<Marker> marker = markers->at(i);
        msgs << marker->get_id() << ' ';
    }
    msgs << '\n';

    if(num_detected_markers < min_markers_required)
    {
        Logger::Instance()->Error("Did not detect at least {} markers as expected. {}", min_markers_required, msgs.str());

        return false;
    }
    Logger::Instance()->Info("Found min number of markers {}. {}", min_markers_required, msgs.str());

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


    //TODO: adjust path based on hover vs. dispense calibration
    std::string write_path = ini_base_path + "/" + position + "_calibration_image.jpg";
    Logger::Instance()->Info("Saving image to {}", write_path);
    session->refresh();
    cv::imwrite(write_path, *(session->get_color_mat()));

    Logger::Instance()->Info("Captured and wrote image to local device. Running marker detector now.");
    MarkerDetector detector = MarkerDetector(16, 5);


    // 5 attempts at finding and validating markers
    int num_attempts = 10;
    for(int i = 0; i < num_attempts; i++)
    {
        Logger::Instance()->Info("Marker Detector attempt: {}", i);
        std::shared_ptr<std::vector<std::shared_ptr<Marker>>> detected_markers = detector.detect_markers(session->get_color_mat());
        bool validation_check = validate_markers(session,detected_markers, camera_points, calibration_xy, marker_ids); //validates depth and within search region
        if(validation_check) break; //break out of loop if all markers were detected and valid
        if(i == num_attempts - 1) return false;
        session->refresh(); //take a new image to try again
    }

    Logger::Instance()->Info("Markers all found to be valid, and camera coordinates for calibration have been found.");
    std::vector<int> id_vec;
    return true;
}

bool try_camera_calibration_cycle(std::shared_ptr<fulfil::depthcam::Session> session,
                                  const std::string& ini_write_path, calibration_type tray_position){
    std::shared_ptr<std::string> serial_number = session->get_serial_number();
    std::cout << "We will begin calibrating camera serial no. " << *serial_number << std::endl;

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
    }
    std::cout << "position " << position << std::endl;


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

        write_calibration_to_ini(ini_write_path, serial_number, calibration_xy, camera_points, marker_ids, tray_position);
        Logger::Instance()->Info("Calibration was a success!!!");
        return 1;
    }

    Logger::Instance()->Error("Issue getting calibration. Nothing will be written to file.");
    return 0;
}


int main(int argc, char** argv)
{
    std::cout << "STARTING" << std::endl;

    Logger::Instance()->SetConsoleLogLevel(Logger::Level::Trace);
    Logger::Instance()->SetFileLogLevel(Logger::Level::Debug);

    calibration_args calibration_params{};
    calibration_params.parse_args(argc, argv);

    std::cout << "Position: " << TrayPositionNames[calibration_params.lift_height]  << std::endl;
    std::cout << "Min tags: " << calibration_params.min_tags  << std::endl;
    std::cout << "Output: " << calibration_params.output_dir  << std::endl;



    std::cout << "Calibration will be performed on tray camera" << std::endl;

    try {
        std::string ini = calibration_params.output_dir / calibration_filename(TrayPositionNames[calibration_params.lift_height]);
        Logger::Instance()->Info("Wiping file: {}", ini);
        wipe_file(ini);

        // Getting the tray session
        ini_base_path = calibration_params.output_dir;
        min_markers_required = calibration_params.min_tags;
        try_camera_calibration_cycle(calibration_params.session, ini, calibration_params.lift_height);
    } catch (const std::exception& ex){
        Logger::Instance()->Error("Exception thrown during routine!\n{}", ex.what());
    }

    return 0;
}
