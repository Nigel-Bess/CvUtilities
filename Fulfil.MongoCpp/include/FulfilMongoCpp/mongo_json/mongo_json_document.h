//
// Created by amber on 7/26/20.
//

#ifndef MONGO_JSON_DOCUMENT_H
#define MONGO_JSON_DOCUMENT_H

#include <fstream>
#include <memory>

#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <FulfilMongoCpp/mongo_json/json.hpp>
#include <FulfilMongoCpp/mongo_objects/mongo_document.h>
#include <FulfilMongoCpp/mongo_objects/mongo_object_id.h>
#include <FulfilMongoCpp/mongo_objects/mongo_element.h>
#include <FulfilMongoCpp/mongo_objects/mongo_generic_document.h>




namespace ff_mongo_cpp::mongo_objects {
        class MongoJsonDocument : public MongoGenericDocument {
        public:
            MongoJsonDocument(bsoncxx::document::value doc,
                    const std::string& collection_name, const std::string& db_name);
            MongoJsonDocument(bsoncxx::document::view doc,
                              const std::string& collection_name, const std::string& db_name);
            MongoJsonDocument(nlohmann::json doc,
                              const std::string& collection_name, const std::string& db_name);
            MongoJsonDocument(const std::string& doc,
                    const std::string& collection_name, const std::string& db_name);

            MongoJsonDocument();

            ~MongoJsonDocument() = default;

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
            }

            virtual void update_values(bsoncxx::document::view doc);

            virtual void to_file(const std::string &filename) {
              std::ofstream file(filename);
              file << this->str();
            }


            virtual std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> MakeNewMongoDocument(bsoncxx::document::view doc,
                                          const std::string& collection_name, const std::string& db_name);

            virtual std::shared_ptr<MongoGenericDocument> MakeNewGenericDocument(bsoncxx::document::view doc,
                                       const std::string& collection_name, const std::string& db_name);

            virtual bool empty() const;

            virtual bsoncxx::document::value MakeWritableValue();

            nlohmann::json data;


        protected:
            int parse_in_values(bsoncxx::document::view doc);


        private:
            std::string collection_name;
            std::shared_ptr<bsoncxx::document::value> document_data;
            std::string db_name;
            ff_mongo_cpp::mongo_objects::MongoObjectID oid;
            int parse_state;

        };
    }
#endif //MONGO_JSON_DOCUMENT_H


