//
// Created by amber on 7/26/20.
//

#ifndef FULFIL_MONGO_CPP_BASE_MONGO_CONNECTION_H_
#define FULFIL_MONGO_CPP_BASE_MONGO_CONNECTION_H_

#include <iostream>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include "FulfilMongoCpp/mongo_filter/mongo_filters.h"
#include "FulfilMongoCpp/mongo_objects/mongo_document.h"
#include <mongocxx/exception/exception.hpp>
#include <bsoncxx/exception/exception.hpp>
#include "FulfilMongoCpp/mongo_objects/mongo_generic_document.h"
using ff_mongo_cpp::mongo_objects::MongoGenericDocument;


namespace ff_mongo_cpp {
    // collection getter
    class BaseMongoConnection {
    public:
        explicit BaseMongoConnection(const std::string& conn_string);

        // Find functions
        mongocxx::cursor findAll(const std::string &DBName, const std::string &collection_name);

        // ObjectId based finds
        mongocxx::cursor findManyByObjectIDs(const std::string &DBName, const std::string &collection_name, const std::vector<bsoncxx::oid> &oids);
        mongocxx::cursor findManyByObjectIDs(mongocxx::collection collection, const std::vector<bsoncxx::oid> &oids);
        core::optional<bsoncxx::document::value>
        findOneByObjectID(const std::string &DBName, const std::string &collection_name, const bsoncxx::oid &oid);
        bsoncxx::stdx::optional<bsoncxx::document::value>
        findOneByFilter(const std::string &DBName, const std::string &collection_name, bsoncxx::document::view filter);
        // Filter finds
        mongocxx::cursor findManyByFilter(const std::string &DBName, const std::string &collection_name, bsoncxx::document::view filter);


        bool tryGetCollection(const std::string &DBName, const std::string &collection_name, mongocxx::collection &collection);

        bool tryInsertOne(const std::string &DBName, const std::string &collection_name,
                          bsoncxx::document::view doc,
                          bsoncxx::oid &insert_id);


        int tryInsertMany(const std::string &DBName, const std::string &collection_name,
                          std::shared_ptr<std::vector<bsoncxx::document::value>> inserts,
                          std::vector<bsoncxx::oid> &insert_ids);

        // return true if successfully found a database entry matching the filter and updated
        bool tryUpdateOneByFilter(const std::string &DBName, const std::string &collection_name,
                                  bsoncxx::document::view filter,
                                  bsoncxx::document::view doc, const std::string &update_type = "$set");


        bool tryUpdateOneByObjectID(const std::string &DBName, const std::string &collection_name, bsoncxx::document::view doc,
                                    bsoncxx::oid oid, const std::string &update_type = "$set");

        int tryUpdateManyByFilter(const std::string &DBName, const std::string &collection_name,
                                  bsoncxx::document::view filter,
                                  bsoncxx::document::view doc, const std::string &update_type = "$set");

        int tryUpdateManyByObjectIDs(const std::string &DBName, const std::string &collection_name, bsoncxx::document::view doc,
                                     const std::vector<bsoncxx::oid> &oids, const std::string &update_type = "$set");

        bool tryDeleteOneByFilter(const std::string &DBName, const std::string &collection_name,
                                  bsoncxx::document::view filter);

        bool tryDeleteOneByObjectID(const std::string &DBName, const std::string &collection_name,
                                    bsoncxx::oid oid);

        int tryDeleteManyByFilter(const std::string &DBName, const std::string &collection_name,
                                  bsoncxx::document::view filter);

        int tryDeleteManyByObjectIDs(const std::string &DBName, const std::string &collection_name,
                                     const std::vector<bsoncxx::oid> &oids);

        /**
         * Document conversion to Fulfil types
         * */

        static void CursorToMongoDocument(mongocxx::cursor &&cursor,
                                   const std::string& collection_name, const std::string& db_name,
                                   std::shared_ptr<std::vector<std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument>>>& mongo_docs,
                                   std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> template_doc);

        static void CursorToMongoDocument(mongocxx::cursor &cursor,
                                   const std::string& collection_name, const std::string& db_name,
                                   std::shared_ptr<std::vector<std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument>>>& mongo_docs,
                                   std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> template_doc);

        static void CursorToMongoDocument(mongocxx::cursor &&cursor,
                                   const std::string& collection_name, const std::string& db_name,
                                   std::shared_ptr<std::vector<std::shared_ptr<MongoGenericDocument>>>& mongo_docs,
                                   std::shared_ptr<MongoGenericDocument> template_doc);

        static void CursorToMongoDocument(mongocxx::cursor &cursor,
                                   const std::string& collection_name, const std::string& db_name,
                                   std::shared_ptr<std::vector<std::shared_ptr<MongoGenericDocument>>>& mongo_docs,
                                   std::shared_ptr<MongoGenericDocument> template_doc);

        /**
         * Debug / IO
         * */
        static void PrintQueryOutput(mongocxx::cursor &cursor);

        static void PrintQueryOutput(mongocxx::cursor &&cursor);

        static void PrintQueryOutput(bsoncxx::stdx::optional<bsoncxx::document::value> doc);

        bool IsConnected() const;

    protected:
        static bsoncxx::document::value makeBsonObjectIDArrayQuery(const std::vector<bsoncxx::oid>& values, const std::string& query_type = "$in");

    private:
        bool _connected;
        mongocxx::client _client;
        std::vector<std::string> avaliable_db;





    };
}
#endif //FULFIL_MONGO_CPP_BASE_MONGO_CONNECTION_H_
