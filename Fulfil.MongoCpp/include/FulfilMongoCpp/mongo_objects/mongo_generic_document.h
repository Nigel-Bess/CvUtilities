//
// Created by amber on 7/26/20.
//

#ifndef FULFILMONGOCPP_MONGO_GENERIC_DOCUMENT_H_
#define FULFILMONGOCPP_MONGO_GENERIC_DOCUMENT_H_

#include <fstream>


#include <memory>

#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>

#include "FulfilMongoCpp/mongo_objects/mongo_object_id.h"
#include "FulfilMongoCpp/mongo_objects/mongo_element.h"
#include "FulfilMongoCpp/mongo_objects/mongo_document.h"

using ff_mongo_cpp::mongo_objects::MongoElement;


namespace ff_mongo_cpp {
    namespace mongo_objects {
        class MongoGenericDocument : public MongoDocument {
        public:

            virtual ~MongoGenericDocument() = default;

            //Todo if I get around to making the JSon getter interface, it can be added here

            virtual std::string str() const = 0;

            virtual operator std::string() const = 0;

            virtual void to_file(const std::string &filename) = 0;

            virtual std::shared_ptr<MongoGenericDocument> MakeNewGenericDocument(bsoncxx::document::view doc,
                          const std::string& collection_name, const std::string& db_name) = 0;
            virtual void update_values(bsoncxx::document::view doc) = 0;

            virtual bool empty() const = 0;

        };
    }
}
#endif //FULFILMONGOCPP_MONGO_GENERIC_DOCUMENT_H_


