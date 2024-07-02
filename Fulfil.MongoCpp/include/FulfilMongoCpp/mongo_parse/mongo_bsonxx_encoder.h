//
// Created by amber on 8/12/20.
//

#ifndef FULFILMONGOCPP_MONGO_BSONXX_ENCODER_H_
#define FULFILMONGOCPP_MONGO_BSONXX_ENCODER_H_

#include <bsoncxx/json.hpp>
#include <utility>
#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/types.hpp>

#include "FulfilMongoCpp/mongo_objects/mongo_object_id.h"

#include "FulfilMongoCpp/mongo_filter/mongo_filters.h"

namespace ff_mongo_cpp {
  namespace mongo_parse {
    class MongoBsonxxEncoder {
      public:

        template<typename T>
        void bulkAddField(std::vector<std::string> keys, std::vector<T> values)
        {
          int num_kv = keys.size();
          if (num_kv != values.size())
            throw std::runtime_error("Mismatch in number of keys and values in bulkAddField!");
          for (int i = 0; i < num_kv; i++){
            addField(keys[i], values[i]);
          }
        }

        template<typename T>
        void addField(std::string key, T&& value) {
          using bsoncxx::builder::basic::kvp;
          master_doc_builder.append(kvp(key, convert_bson(value)));
        }

        template<typename T>
        void addField(std::string key, std::vector<T> values) {
          using bsoncxx::builder::basic::kvp;
          bsoncxx::array::value arr_value = makeBsonArray(values);
          master_doc_builder.append(kvp(key, bsoncxx::types::b_array{ arr_value.view() }));
        }

        void addCurrentTimeField(std::string key);

        void concatenate(bsoncxx::document::view value);

        bsoncxx::document::value extract();


      private:
        bsoncxx::builder::basic::document master_doc_builder{};

        template<typename T>
        bsoncxx::array::value makeBsonArray(std::vector<T> values) {
          auto arr_builder = bsoncxx::builder::basic::array{};
          for (auto &val : values)
            arr_builder.append(convert_bson(val));
          return arr_builder.extract();
        }

        template<typename T>
        T convert_bson(T&& value){
          return value;
        };

        template<typename T>
        bsoncxx::array::value convert_bson(std::vector<T> value){
          return makeBsonArray(value);
        };

        bsoncxx::types::b_oid convert_bson(ff_mongo_cpp::mongo_objects::MongoObjectID value);
        bsoncxx::types::b_array convert_bson(bsoncxx::array::view value);
        bsoncxx::types::b_document convert_bson(bsoncxx::document::view value);
        bsoncxx::types::b_document convert_bson(ff_mongo_cpp::mongo_filter::MongoFilter value);

    };
  }
}

#endif //FULFILMONGOCPP_MONGO_BSONXX_ENCODER_H_
