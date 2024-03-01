//
// Created by amber on 7/29/20.
//

#ifndef FULFILMONGOCPP_MONGO_BASIC_DOCUMENT_H
#define FULFILMONGOCPP_MONGO_BASIC_DOCUMENT_H

#include <fstream>

#include <memory>

#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/stdx/string_view.hpp>

#include <FulfilMongoCpp/mongo_objects/mongo_object_id.h>
#include <FulfilMongoCpp/mongo_objects/mongo_element.h>
#include <FulfilMongoCpp/mongo_parse/mongo_bsonxx_encoder.h>
#include <FulfilMongoCpp/mongo_parse/mongo_bsonxx_parser.h>

#include <FulfilMongoCpp/mongo_objects/mongo_document.h>
#include <FulfilMongoCpp/mongo_objects/mongo_generic_document.h>



namespace ff_mongo_cpp {
    namespace mongo_objects {
        class MongoBasicDocument : public MongoGenericDocument {
        public:
            MongoBasicDocument(bsoncxx::document::value doc,
                              const std::string& collection_name, const std::string& db_name);
            MongoBasicDocument(ff_mongo_cpp::mongo_parse::MongoBsonxxEncoder& doc_encoder,
                               const std::string& collection_name, const std::string& db_name);
            MongoBasicDocument(bsoncxx::document::view doc,
                              const std::string& collection_name, const std::string& db_name);
            MongoBasicDocument();

            ~MongoBasicDocument() = default;

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

            virtual bsoncxx::document::value MakeWritableValue();

            virtual void to_file(const std::string &filename) {
              std::ofstream file(filename);
              file << this->str();
            }


            virtual std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> MakeNewMongoDocument(bsoncxx::document::view doc,
                    const std::string& collection_name, const std::string& db_name);

            virtual std::shared_ptr<MongoGenericDocument> MakeNewGenericDocument(bsoncxx::document::view doc,
                    const std::string& collection_name, const std::string& db_name);

            virtual bool empty() const;

            template<typename... Args>
            bool parseFromRoot(Args&&... args)
            {
              return this->parser.parseFromRoot(std::forward<Args>(args)...);
            }

            template<typename T, typename... Args>
            T getFromRoot(Args&&... args) {
              return this->parser.getFromRoot<T>(std::forward<Args>(args)...);
            }


        protected:
            int parse_in_values(bsoncxx::document::view doc);


        private:
            std::shared_ptr<bsoncxx::document::value> document_data;
            std::string collection_name;
            ff_mongo_cpp::mongo_parse::MongoBsonxxParser parser;
            std::string db_name;
            ff_mongo_cpp::mongo_objects::MongoObjectID oid;
            int parse_state;


        };
    }
}

#endif //FULFILMONGOCPP_MONGO_BASIC_DOCUMENT_H
