//
// Created by amber on 10/26/20.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_MONGO_TRAY_RECIPE_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_MONGO_TRAY_RECIPE_H_

#include <iostream>
#include <memory>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

#include <FulfilMongoCpp/mongo_objects/mongo_document.h>
#include <FulfilMongoCpp/mongo_objects/mongo_object_id.h>
#include <FulfilMongoCpp/mongo_objects/mongo_element.h>
#include "Fulfil.Dispense/tray/tray.h"


namespace fulfil {
    namespace mongo
    {
        class MongoTrayRecipe : public ff_mongo_cpp::mongo_objects::MongoDocument
        {
        public:
            MongoTrayRecipe();
            MongoTrayRecipe(bsoncxx::document::value doc);
            MongoTrayRecipe(bsoncxx::document::view doc);

            ~MongoTrayRecipe() = default;

            ff_mongo_cpp::mongo_objects::MongoObjectID GetObjectID();
            std::string GetCollection();
            std::string GetDataBase();
            void SetCollection(const std::string& collection_name);
            void SetDataBase(const std::string& db_name);
            void SetObjectID(ff_mongo_cpp::mongo_objects::MongoObjectID new_id);

            void update_values(bsoncxx::document::view doc);

            bsoncxx::document::value MakeWritableValue();

            std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> MakeNewMongoDocument(bsoncxx::document::view doc,
                                  const std::string& collection_name = "", const std::string& db_name = "");

            std::shared_ptr<fulfil::dispense::tray::Tray> build();


        private:
            int parse_in_values(bsoncxx::document::view doc);
            // General base info
            std::string collection_name;
            std::string db_name;
            ff_mongo_cpp::mongo_objects::MongoObjectID oid;

            // Class specific
            std::string tray_type_name;
            int lane_count;
            std::vector<double> lane_centers;
            double max_item_width;
            double min_item_width;
            // check if Floating raft anywhere else other than in name?

        };

    } // namespace mongo
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_MONGO_TRAY_RECIPE_H_
