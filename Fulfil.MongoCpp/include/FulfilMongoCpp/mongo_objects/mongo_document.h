//
// Created by amber on 7/26/20.
//

#ifndef FULFILMONGOCPP_MONGO_DOCUMENT_H_
#define FULFILMONGOCPP_MONGO_DOCUMENT_H_

#include <fstream>

#include <memory>

#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>

#include "FulfilMongoCpp/mongo_objects/mongo_object_id.h"
#include "FulfilMongoCpp/mongo_objects/mongo_element.h"

using ff_mongo_cpp::mongo_objects::MongoElement;


namespace ff_mongo_cpp {
    namespace mongo_objects {
        class MongoDocument {
        public:

            virtual ~MongoDocument() = default;

            /*
             * Returns a copy of the Object Id associated with the document or the empty string
             * if no oid is currently set.
             * */
            virtual ff_mongo_cpp::mongo_objects::MongoObjectID GetObjectID() = 0;
            virtual std::string GetCollection() = 0;
            virtual std::string GetDataBase() = 0;
            virtual void SetCollection(const std::string& collection_name) = 0;
            virtual void SetDataBase(const std::string& db_name) = 0;
            virtual void SetObjectID(ff_mongo_cpp::mongo_objects::MongoObjectID new_id) = 0;

            virtual bsoncxx::document::value MakeWritableValue() = 0;

            virtual void update_values(bsoncxx::document::view doc) = 0;


            virtual std::shared_ptr<MongoDocument> MakeNewMongoDocument(bsoncxx::document::view doc,
                    const std::string& collection_name, const std::string& db_name) = 0;


        };
    }
}
#endif //FULFILMONGOCPP_MONGO_DOCUMENT_H_


