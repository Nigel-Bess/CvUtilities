//
// Created by amber on 8/14/20.
//

#ifndef FULFILMONGOCPP_MONGO_CONNECTION_H
#define FULFILMONGOCPP_MONGO_CONNECTION_H

#include <iostream>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include "FulfilMongoCpp/mongo_filter/mongo_filters.h"
#include "FulfilMongoCpp/mongo_objects/mongo_document.h"
#include <mongocxx/exception/exception.hpp>
#include <bsoncxx/exception/exception.hpp>
#include <FulfilMongoCpp/mongo_objects/mongo_generic_document.h>

#include "FulfilMongoCpp/base_mongo_connection.h"



namespace ff_mongo_cpp {
    // collection getter
    class MongoConnection : public BaseMongoConnection{
    public:
        explicit MongoConnection(const std::string& conn_string);

        /** Find functions **/
        bool tryFindAll(const std::string &DBName, const std::string &collection_name,
                        std::shared_ptr<std::vector<std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument>>> docs,
                        std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> template_doc);

        // ObjectId based finds
        bool tryFindByObjectIDs(const std::string &DBName, const std::string &collection_name,
                                const std::vector<bsoncxx::oid> &oids, std::shared_ptr<std::vector<std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument>>> docs,
                                std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> template_doc);
        // Worth trying to avoid unnecessary copies on the vec

        bool tryFindByObjectIDs(const std::string &DBName, const std::string &collection_name,
                                const std::vector<ff_mongo_cpp::mongo_objects::MongoObjectID> &oids, std::shared_ptr<std::vector<std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument>>> docs,
                                std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> template_doc);

        bool tryFindByObjectIDs(const std::string &DBName, const std::string &collection_name,
                                const std::vector<std::string> &oids, std::shared_ptr<std::vector<std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument>>> docs,
                                std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> template_doc);

        bool tryFindOneByObjectID(const std::string &DBName, const std::string &collection_name,
                                  ff_mongo_cpp::mongo_objects::MongoObjectID oid, std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> doc);

        // Filter finds
        bool tryFindOneByFilter(const std::string &DBName, const std::string &collection_name,
                                                 mongo_filter::MongoFilter filter,  std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> doc);

        bool tryFindByFilter(const std::string &DBName, const std::string &collection_name, mongo_filter::MongoFilter filter,
                std::shared_ptr<std::vector<std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument>>> docs, std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> template_doc);

        bool tryFindByFilter(const std::string &DBName, const std::string &collection_name, mongo_filter::MongoFilter filter,
                             std::shared_ptr<std::vector<std::shared_ptr<MongoGenericDocument>>> docs, std::shared_ptr<MongoGenericDocument> template_doc);

        /** Insert functions **/
        bool tryInsertOne(const std::string &DBName, const std::string &collection_name,
                          std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> doc,
                          ff_mongo_cpp::mongo_objects::MongoObjectID &insert_id);

        int tryInsertMany(const std::string &DBName, const std::string &collection_name,
                          std::shared_ptr<std::vector<std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument>>> inserts,
                          std::vector<ff_mongo_cpp::mongo_objects::MongoObjectID> &insert_ids);

        bool tryInsertOne(std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> doc,
                          ff_mongo_cpp::mongo_objects::MongoObjectID &insert_id);

        int tryInsertMany(std::shared_ptr<std::vector<std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument>>> inserts,
                          std::vector<ff_mongo_cpp::mongo_objects::MongoObjectID> &insert_ids);

        /** Update functions **/

        bool tryUpdateOneByFilter(const std::string &DBName, const std::string &collection_name,
                                  mongo_filter::MongoFilter filter,
                                  std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> doc, const std::string &update_type = "$set");

        bool tryUpdateOneByFilter(mongo_filter::MongoFilter filter,
                                  std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> doc, const std::string &update_type = "$set");

        int tryUpdateManyByFilter(const std::string &DBName, const std::string &collection_name,
                                  mongo_filter::MongoFilter filter,
                                  std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> doc, const std::string &update_type = "$set");


        bool tryUpdateOneByObjectID(const std::string &DBName, const std::string &collection_name, std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> doc,
                                    ff_mongo_cpp::mongo_objects::MongoObjectID oid, const std::string &update_type = "$set");

        bool tryUpdateOneByObjectID(std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> doc, ff_mongo_cpp::mongo_objects::MongoObjectID oid,
                                      const std::string &update_type = "$set");

        int tryUpdateManyByObjectIDs(const std::string &DBName, const std::string &collection_name, std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> doc,
                                     const std::vector<bsoncxx::oid> &oids, const std::string &update_type = "$set");

        template<typename ID>
        int tryUpdateManyByObjectIDs(const std::string &DBName, const std::string &collection_name, std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> doc,
                                     const std::vector<ID> &oids, const std::string &update_type = "$set")
        {
          return tryUpdateManyByObjectIDs(DBName, collection_name, doc, ff_mongo_cpp::mongo_objects::MongoObjectID::convertToBSonIdVec(oids), update_type);
        }

        /** Delete functions **/
        bool tryDeleteOneByFilter(const std::string &DBName, const std::string &collection_name,
                                  mongo_filter::MongoFilter filter);

        int tryDeleteManyByFilter(const std::string &DBName, const std::string &collection_name,
                                  mongo_filter::MongoFilter filter);


        bool tryDeleteOneByObjectID(const std::string &DBName, const std::string &collection_name,
                                    ff_mongo_cpp::mongo_objects::MongoObjectID oid);


        int tryDeleteManyByObjectIDs(const std::string &DBName, const std::string &collection_name,
                                     const std::vector<std::string> &oids);

        int tryDeleteManyByObjectIDs(const std::string &DBName, const std::string &collection_name,
                                     const std::vector<ff_mongo_cpp::mongo_objects::MongoObjectID> &oids);

    };
}

#endif //FULFILMONGOCPP_MONGO_CONNECTION_H
