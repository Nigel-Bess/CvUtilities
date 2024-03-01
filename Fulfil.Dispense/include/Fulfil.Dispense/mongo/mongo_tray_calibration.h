//
// Created by vincent on 2021-10-22.
//

#ifndef CODE_MONGO_TRAY_CALIBRATION_H
#define CODE_MONGO_TRAY_CALIBRATION_H

#include <Fulfil.CPPUtils/eigen.h>
#include <FulfilMongoCpp/mongo_json/mongo_json_document.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include <FulfilMongoCpp/mongo_connection.h>

namespace fulfil::mongo{
        class MongoTrayCalibration : public ff_mongo_cpp::mongo_objects::MongoDocument
        {

        public:
            MongoTrayCalibration() = default;
            MongoTrayCalibration(std::string machine, std::string serial_number, std::vector<std::pair <int, int>>& calibration_xy,
                                 std::vector<std::shared_ptr<Eigen::Matrix3Xd>>& camera_points, std::vector<float>& tray_x,
                                 std::vector<float>& tray_y, std::vector<float>& tray_z, std::string position);

            ~MongoTrayCalibration() = default;

            ff_mongo_cpp::mongo_objects::MongoObjectID GetObjectID();
            std::string GetCollection();
            std::string GetDataBase();
            void SetCollection(const std::string& collection_name);
            void SetDataBase(const std::string& db_name);

            void SetObjectID(ff_mongo_cpp::mongo_objects::MongoObjectID new_id);

            void update_values(bsoncxx::document::view doc);
            bsoncxx::document::value MakeWritableValue();

            std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> MakeNewMongoDocument(bsoncxx::document::view doc,
                                                                const std::string& collection_name = "",
                                                                const std::string& db_name = "");

            // TODO @Vincent Please Fix. Do not pass bare ptr it is not necessary
            void checkCalibrationExists(std::string serial_num);
            std::vector<std::string> findLastTrayCalibration(std::string vls_id, std::shared_ptr<ff_mongo_cpp::MongoConnection> mongo_conn,
                                                             std::shared_ptr<INIReader> tray_config_reader);
            std::string get_LFB_config(std::shared_ptr<INIReader> reader, std::shared_ptr<ff_mongo_cpp::MongoConnection> mongo_conn);

            std::string machine{};
            std::string serial_number{}; // This is fucking dumb. TODO fix
            std::string position{};

            ff_mongo_cpp::mongo_objects::MongoJsonDocument pixel_locations, camera_coordinates, tray_coordinates;

        private:
            std::string collection_name { "TrayCalibrations"};
            std::string db_name {"Recipes"};
            ff_mongo_cpp::mongo_objects::MongoObjectID oid{};
        };
    }

#endif //CODE_MONGO_TRAY_CALIBRATION_H
