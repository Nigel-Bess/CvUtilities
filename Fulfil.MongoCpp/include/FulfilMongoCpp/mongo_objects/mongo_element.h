//
// Created by amber on 7/27/20.
//FULFIL_MONGO_CPP_BASE_MONGO_CONNECTION_H_

#ifndef FULFIL_MONGO_CPP_OBJECTS_ELEMENT_H_
#define FULFIL_MONGO_CPP_OBJECTS_ELEMENT_H_
#include <iostream>
#include <sstream>
#include <memory>
#include <utility>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>



#include "FulfilMongoCpp/mongo_objects/mongo_object_id.h"




namespace ff_mongo_cpp {
    namespace mongo_objects {
        class MongoElement {
        public:
            virtual int tryGetValue(std::vector<std::string>& container) = 0;
            virtual int tryGetValue(std::vector<int>& container) = 0;
            virtual int tryGetValue(std::vector<double>& container) = 0;
            virtual int tryGetValue(std::vector<bool>& container) = 0;
            virtual int tryGetValue(std::vector<ff_mongo_cpp::mongo_objects::MongoObjectID>& container) = 0;

            virtual int tryGetValue(std::string& container) = 0;
            virtual int tryGetValue(int& container) = 0;
            virtual int tryGetValue(double& container) = 0;
            virtual int tryGetValue(bool& container) = 0;
            virtual int tryGetValue(ff_mongo_cpp::mongo_objects::MongoObjectID& container) = 0;

            virtual bsoncxx::document::view getSubDocView() = 0;

            virtual std::shared_ptr<MongoElement> getNext(const std::string& index) = 0;
            virtual std::shared_ptr<MongoElement> getNext(int index) = 0;

            virtual std::string type() = 0;
            virtual std::string key() = 0;
        protected:
            // TODO ensure not founds properly handled
            //From array class
            bool tryGetElem(const bsoncxx::array::element& element, std::string& container);
            bool tryGetElem(const bsoncxx::array::element& element, int& container);
            bool tryGetElem(const bsoncxx::array::element& element, double& container);
            bool tryGetElem(const bsoncxx::array::element& element, bool& container);
            bool tryGetElem(const bsoncxx::array::element& element, ff_mongo_cpp::mongo_objects::MongoObjectID& container);

            bool tryGetElem(const bsoncxx::document::element& element, std::string& container);
            bool tryGetElem(const bsoncxx::document::element& element, int& container);
            bool tryGetElem(const bsoncxx::document::element& element, double& container);
            bool tryGetElem(const bsoncxx::document::element& element, bool& container);
            bool tryGetElem(const bsoncxx::document::element& element, ff_mongo_cpp::mongo_objects::MongoObjectID& container);



        };
    }
}

#endif //FULFIL_MONGO_CPP_OBJECTS_ELEMENT_H_
