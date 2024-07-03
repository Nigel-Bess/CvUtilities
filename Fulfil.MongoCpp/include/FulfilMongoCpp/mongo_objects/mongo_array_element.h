//
// Created by amber on 7/27/20.
//FULFIL_MONGO_CPP_BASE_MONGO_CONNECTION_H_

#ifndef FULFIL_MONGO_CPP_OBJECTS_ARRAY_ELEMENT_H_
#define FULFIL_MONGO_CPP_OBJECTS_ARRAY_ELEMENT_H_
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


#include "FulfilMongoCpp/mongo_objects/mongo_element.h"

namespace ff_mongo_cpp {
    namespace mongo_objects {
        class MongoArrayElement : public ff_mongo_cpp::mongo_objects::MongoElement{
        public:

            MongoArrayElement(bsoncxx::array::element elem_val);

            int tryGetValue(std::vector<std::string>& container) override;
            int tryGetValue(std::vector<int>& container) override;
            int tryGetValue(std::vector<double>& container) override;
            int tryGetValue(std::vector<bool>& container) override;
            int tryGetValue(std::vector<ff_mongo_cpp::mongo_objects::MongoObjectID>& container) override;

            int tryGetValue(std::string& container) override;
            int tryGetValue(int& container) override;
            int tryGetValue(double& container) override;
            int tryGetValue(bool& container) override;
            int tryGetValue(ff_mongo_cpp::mongo_objects::MongoObjectID& container) override;

            std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoElement> getNext(const std::string& index) override;
            std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoElement> getNext(int index) override;

            bsoncxx::document::view getSubDocView() override {
              if(this->raw_element.type() == bsoncxx::type::k_document)
                return this->raw_element.get_document().view();
              return bsoncxx::document::view();
            }

            std::string type() override;
            std::string key() override { return ""; }

            template<typename T>
            T get() {
              T value;
              if (!ff_mongo_cpp::mongo_objects::MongoElement::tryGetElem(this->raw_element, value)) {
                throw std::runtime_error("Tried to get an incorrect value type from element!");
              }
              return value;
            }

        private:
            template<typename T>
            bool tryGetValueHelper(std::vector<T>& container)
            {
              if (this->raw_element && this->raw_element.type() != bsoncxx::v_noabi::type::k_array) return 0;
              bsoncxx::array::view arr = bsoncxx::array::view(this->raw_element.get_array());
              for (const bsoncxx::array::element& a : arr ){
                T elem;
                ff_mongo_cpp::mongo_objects::MongoElement::tryGetElem(a, elem);
                container.push_back(elem);
              }
              return 1;
            }

            bsoncxx::array::element raw_element;

        };
    }
}

#endif //FULFIL_MONGO_CPP_OBJECTS_ARRAY_ELEMENT_H_
