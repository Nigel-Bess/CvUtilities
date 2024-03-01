//
// Created by amber on 7/29/20.
//

#ifndef FULFILMONGOCPP_MONGO_SCOPED_DOCUMENT_H
#define FULFILMONGOCPP_MONGO_SCOPED_DOCUMENT_H
#include <fstream>
#include <memory>

#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>


#include "FulfilMongoCpp/mongo_objects/mongo_document.h"
#include "FulfilMongoCpp/mongo_objects/mongo_object_id.h"
#include "FulfilMongoCpp/mongo_objects/mongo_element.h"
#include "FulfilMongoCpp/mongo_parse/mongo_bsonxx_parser.h"
using ff_mongo_cpp::mongo_parse::MongoBsonxxParser;


#include "FulfilMongoCpp/mongo_objects/mongo_generic_document.h"
using ff_mongo_cpp::mongo_objects::MongoGenericDocument;


namespace ff_mongo_cpp {
    namespace mongo_objects {
        class MongoScopedDocument : public MongoGenericDocument {
        public:
            MongoScopedDocument(bsoncxx::document::value doc,
                               const std::string& collection_name, const std::string& db_name);
            MongoScopedDocument(bsoncxx::document::view doc,
                               const std::string& collection_name, const std::string& db_name);
            MongoScopedDocument();

            ~MongoScopedDocument() = default;

            /*
             * Returns a copy of the Object Id associated with the document or the empty string
             * if no oid is currently set.
             * */
            ff_mongo_cpp::mongo_objects::MongoObjectID GetObjectID();
            std::string GetCollection();
            std::string GetDataBase();
            void SetCollection(const std::string& collection_name);
            void SetDataBase(const std::string& db_name);
            void SetObjectID(ff_mongo_cpp::mongo_objects::MongoObjectID new_id);

            virtual std::string str() const;

            operator std::string() const {
              return this->str();
            };

            virtual void to_file(const std::string &filename) {
              std::ofstream file(filename);
              file << this->str();
            };

            virtual void update_values(bsoncxx::document::view doc);

            virtual bsoncxx::document::value MakeWritableValue();

            virtual std::shared_ptr<MongoDocument> MakeNewMongoDocument(bsoncxx::document::view doc,
                      const std::string& collection_name, const std::string& db_name);

            virtual std::shared_ptr<MongoGenericDocument> MakeNewGenericDocument(bsoncxx::document::view doc,
                        const std::string& collection_name, const std::string& db_name);

            virtual bool empty() const;

            template<typename... Args>
            bool parseFromRoot(Args&&... args)
            {
              std::cout << "in parse from root" << std::endl;
              return this->parser.parseFromRoot(std::forward<Args>(args)...);
            }

            template<typename T, typename... Args>
            T getFromRoot(Args&&... args) {
              return this->parser.getFromRoot<T>(std::forward<Args>(args)...);
            }


        private:
            bsoncxx::document::view document_data;
            MongoBsonxxParser parser;
            std::string collection_name;
            std::string db_name;
            ff_mongo_cpp::mongo_objects::MongoObjectID oid;

        };
    }
}
#endif //FULFILMONGOCPP_MONGO_SCOPED_DOCUMENT_H
