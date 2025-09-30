//
// Created by vincent on 2021-10-22.
//
#include "Fulfil.Dispense/mongo/mongo_tray_calibration.h"
#include <FulfilMongoCpp/mongo_parse/mongo_bsonxx_encoder.h>
#include <Fulfil.CPPUtils/inih/ini_utils.h>


using fulfil::mongo::MongoTrayCalibration;
using ff_mongo_cpp::mongo_parse::MongoBsonxxEncoder;
using ff_mongo_cpp::mongo_filter::MongoFilter;
using ff_mongo_cpp::MongoConnection;
using ff_mongo_cpp::mongo_objects::MongoObjectID;
using ff_mongo_cpp::mongo_objects::MongoDocument;
using ff_mongo_cpp::mongo_objects::MongoJsonDocument;
using fulfil::utils::Logger;


MongoTrayCalibration::MongoTrayCalibration(std::string machine, std::string serial_number,
                                           std::vector<std::pair <int, int>>& calibration_xy,
                                           std::vector<std::shared_ptr<Eigen::Matrix3Xd>>& camera_points,
                                           std::vector<float>& tray_x, std::vector<float>& tray_y, std::vector<float>& tray_z,
                                           std::string position){

    this->collection_name = "TrayCalibrations";
    this->db_name = "Recipes";

    this->machine = machine;
    this->serial_number = serial_number;
    this->position = position;

    nlohmann::json pixel_locations_temp, camera_coordinates_temp, tray_coordinates_temp;

    std::vector<int> x, y, z;
    for (int i = 0; i < calibration_xy.size(); i++){
        x.push_back(calibration_xy[i].first);
        y.push_back(calibration_xy[i].second);
        z.push_back(0);
    }

    pixel_locations_temp["x"] = x;
    pixel_locations_temp["y"] = y;
    pixel_locations_temp["z"] = z;

    this->pixel_locations = MongoJsonDocument(pixel_locations_temp, "TrayCalibrations", "Recipes");

    std::vector<double> x_f, y_f, z_f;
    auto unzip_point = [&x_f, &y_f, &z_f](std::shared_ptr<Eigen::Matrix3Xd> point) {
        x_f.push_back(((*point)(0,0)));
        y_f.push_back(((*point)(1,0)));
        z_f.push_back(((*point)(2,0))); };
    std::for_each( camera_points.begin(), camera_points.end(), unzip_point);
    camera_coordinates_temp["x"] = x_f;
    camera_coordinates_temp["y"] = y_f;
    camera_coordinates_temp["z"] = z_f;

    this->camera_coordinates = MongoJsonDocument(camera_coordinates_temp, "TrayCalibrations", "Recipes");

    tray_coordinates_temp["x"] = tray_x;
    tray_coordinates_temp["y"] = tray_y;
    tray_coordinates_temp["z"] = tray_z;

    this->tray_coordinates = MongoJsonDocument(tray_coordinates_temp, "TrayCalibrations", "Recipes");

}

bsoncxx::document::value MongoTrayCalibration::MakeWritableValue() {

    MongoBsonxxEncoder calibration_encoder = MongoBsonxxEncoder();

    calibration_encoder.addCurrentTimeField("calibration_timestamp");
    calibration_encoder.addField("machine_hostname", this->machine);
    calibration_encoder.addField("serial_number", this->serial_number);
    calibration_encoder.addField("tray_position", this->position);

    calibration_encoder.addField("pixel_locations", this->pixel_locations.MakeWritableValue().view());
    calibration_encoder.addField("camera_coordinates", this->camera_coordinates.MakeWritableValue().view());
    calibration_encoder.addField("tray_coordinates", this->tray_coordinates.MakeWritableValue().view());

    bsoncxx::document::value final_doc = calibration_encoder.extract();
    return final_doc;
}

void MongoTrayCalibration::checkCalibrationExists(std::string serial_num)
{
    //check tray cameras have been calibrated
    auto ini_parse_log = [] (std::string_view filename) {
        Logger::Instance()->Fatal("Failure to read, parse and append {} in directory {}, check path.", filename, INIReader::get_compiled_default_dir_prefix());
    };

    // TODO this is real fuckin dumb. Fix

    const std::string tray_calib_data = std::string("tray_calibration_data").append(serial_num).append("_");

    std::shared_ptr<INIReader> cam_calib_reader = std::make_shared<INIReader>(tray_calib_data + "dispense.ini", true);
    fulfil::utils::ini::validate_ini_parse(*cam_calib_reader, tray_calib_data + "dispense.ini", ini_parse_log);

    cam_calib_reader->appendReader(INIReader(tray_calib_data + "hover.ini", true));
    fulfil::utils::ini::validate_ini_parse(*cam_calib_reader, tray_calib_data + "hover.ini", ini_parse_log);


    cam_calib_reader->appendReader(INIReader("tray_calibration_data_tongue.ini", true));
    fulfil::utils::ini::validate_ini_parse(*cam_calib_reader, "tray_calibration_data_tongue.ini", ini_parse_log);


    std::string dispense_pixel = serial_num + "_pixel_locations";
    std::string dispense_camera = serial_num + "_camera_coordinates";
    std::string dispense_tray = serial_num + "_tray_coordinates";

    std::string hover_pixel = serial_num + "_hover_pixel_locations";
    std::string hover_camera = serial_num + "_hover_camera_coordinates";
    std::string hover_tray = serial_num + "_hover_tray_coordinates";

    std::string tongue_pixel = serial_num + "_tongue_pixel_locations";
    std::string tongue_camera = serial_num + "_tongue_camera_coordinates";
    std::string tongue_tray = serial_num + "_tongue_tray_coordinates";

    std::set<std::string> sections = cam_calib_reader->Sections();

    if (sections.count(dispense_pixel) == 0 || sections.count(dispense_camera) == 0 || sections.count(dispense_tray) == 0){
        Logger::Instance()->Fatal("Tray Camera 1 is missing Dispense Calibrations");
    }
    if (sections.count(hover_pixel) == 0 || sections.count(hover_camera) == 0 || sections.count(hover_tray) == 0){
        Logger::Instance()->Fatal("Tray Camera 1 is missing Hover Calibrations");
    }
    if (sections.count(tongue_pixel) == 0 || sections.count(tongue_camera) == 0 || sections.count(tongue_tray) == 0){

        Logger::Instance()->Fatal("Tray Camera 1 is missing Tongue Calibrations");
    }


}

std::vector<std::string> MongoTrayCalibration::findLastTrayCalibration(std::string vls_id, std::shared_ptr<MongoConnection> mongo_conn,
                                                   std::shared_ptr<INIReader> tray_config_reader){
    MongoObjectID search_id = MongoObjectID(vls_id);
    MongoFilter calib_filter = MongoFilter("VLS", MongoFilter::FilterType::Eq, search_id);
    std::shared_ptr<MongoJsonDocument> calib_doc = std::make_shared<MongoJsonDocument>();
    std::string serial_number;
    if (!mongo_conn->tryFindOneByFilter("Machines", "DepthCameras", calib_filter, calib_doc)){
        fulfil::utils::Logger::Instance()->Fatal("Failed to find VLS ID in Mongo. Check Machines.Depthcameras collection.");
        throw std::runtime_error("Couldn't find VLS in Mongo collection Machines.DepthCameras");
    }
    else{
        std::string last_dispense_calibration, last_hover_calibration, last_tongue_calibration;
        try{
            last_dispense_calibration = calib_doc->data["last_tray_camera_calibration_dispense"]["$oid"];
            last_hover_calibration = calib_doc->data["last_tray_camera_calibration_hover"]["$oid"];
            last_tongue_calibration = calib_doc->data["last_tray_camera_calibration_tongue"]["$oid"];
            serial_number = calib_doc->data["tray_camera_serial_number"];
        }
        catch(const std::exception & e){
            Logger::Instance()->Fatal("Getting last_tray_camera_calibration from matching Mongo document in "
                                      "Machines.Depthcameras failed. Check field exists.");
            throw std::runtime_error("Couldn't find last tray camera calibration in matching Machines.DepthCameras Mongo document");
        }
        if (last_dispense_calibration != tray_config_reader->Get("config_details", serial_number+"_recipe_id_dispense",
           "-1") || last_hover_calibration != tray_config_reader->Get("config_details", serial_number+"_recipe_id_hover", "-1")
           || last_tongue_calibration != tray_config_reader->Get("config_details", serial_number+"_recipe_id_tongue", "-1")){

            Logger::Instance()->Fatal("calibration_id mismatch between local calibration config files and Mongo, check if more"
                                      " recent calibration data available either locally or on Mongo");
            throw std::runtime_error("Mismatch in calibration id between local config file and Mongo");
        }
        std::vector<std::string> ret;
        ret.push_back(last_hover_calibration);
        ret.push_back(last_tongue_calibration);
        ret.push_back(last_dispense_calibration);
        return ret;
    }
}

std::string MongoTrayCalibration::get_LFB_config(std::shared_ptr<INIReader> reader, std::shared_ptr<MongoConnection> mongo_conn){

    MongoObjectID vls_id = MongoObjectID(reader->Get("device_specific", "vls0_id", "000000000000000000000000"));
    MongoFilter searcher = MongoFilter("VLS", MongoFilter::FilterType::Eq, vls_id);
    std::shared_ptr<MongoJsonDocument> doc = std::make_shared<MongoJsonDocument>();
    std::string LFB_config_type;
    if (!mongo_conn->tryFindOneByFilter("Machines", "DepthCameras", searcher, doc)){
        fulfil::utils::Logger::Instance()->Fatal("Failed to find VLS ID in Mongo. Check Machines.Depthcameras collection.");
        throw std::runtime_error("Couldn't find VLS in Mongo collection Machines.DepthCameras");
    }
    else{
        try{
            LFB_config_type = doc->data["LFB_config_type"];
        }
        catch(const std::exception & e){
            Logger::Instance()->Fatal("Getting LFB_config_type from matching Mongo document in "
                                      "Machines.Depthcameras failed. Check field exists.");
            throw std::runtime_error("Couldn't find last LFB config type in matching Machines.DepthCameras Mongo document");
        }
        if (LFB_config_type != "1" && LFB_config_type != "2" && LFB_config_type != "1b" && LFB_config_type != "3"){
            Logger::Instance()->Fatal("Invalid LFB_config_type found - must be one of: 1, 1b, 2, 3");
            throw std::runtime_error("Invalid LFB_config_type found in Mongo");
        }
    }
    return LFB_config_type;
}

void MongoTrayCalibration::SetObjectID(MongoObjectID oid)
{
    this->oid = oid;
}

MongoObjectID MongoTrayCalibration::GetObjectID()
{
    return this->oid;
}

std::string MongoTrayCalibration::GetCollection()
{
    return this->collection_name;
}

std::string MongoTrayCalibration::GetDataBase()
{
    return this->db_name;
}

void MongoTrayCalibration::SetCollection(const std::string& new_collection_name)
{
    this->collection_name = new_collection_name;
}

void MongoTrayCalibration::SetDataBase(const std::string& new_db_name)
{
    this->db_name = new_db_name;
}

std::shared_ptr<MongoDocument> MongoTrayCalibration::MakeNewMongoDocument(bsoncxx::document::view doc,
                                                                   const std::string& collection_name,
                                                                   const std::string& db_name)
{
    std::shared_ptr<MongoTrayCalibration> new_doc = std::make_shared<MongoTrayCalibration>();
    return new_doc;
}


void MongoTrayCalibration::update_values(bsoncxx::document::view doc){}
