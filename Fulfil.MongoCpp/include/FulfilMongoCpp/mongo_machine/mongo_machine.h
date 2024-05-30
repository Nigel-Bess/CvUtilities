//
// Created by amber on 8/13/20.
//

#ifndef FULFILMONGOCPP_MONGO_MACHINE_H
#define FULFILMONGOCPP_MONGO_MACHINE_H

#include <memory>

#include "FulfilMongoCpp/mongo_objects/mongo_object_id.h"
#include "FulfilMongoCpp/mongo_objects/mongo_document.h"
#include "FulfilMongoCpp/mongo_parse/mongo_bsonxx_parser.h"

namespace ff_mongo_cpp {
    namespace mongo_machine {
        class MongoMachine : public ff_mongo_cpp::mongo_objects::MongoDocument
        {
        public:

        protected:

            template<typename T>
            int parse_basic_kvp(bsoncxx::document::view doc,
                                 std::shared_ptr<std::vector<std::string>> keys,
                                std::vector<T*> containers)
            {
                ff_mongo_cpp::mongo_parse::MongoBsonxxParser parser = ff_mongo_cpp::mongo_parse::MongoBsonxxParser(doc);
                std::vector<int> errors;
                parser.bulkParse(keys, containers, errors);
                if (!errors.empty()) {
                    std::cout << "Issue parsing keys ";
                    for (const auto &e : errors)
                        std::cout << keys->at(e) << ", ";
                    std::cout << "please ensure that keys and value types are correct.\n";
                }
                return errors.size();
            }
        private:

    };
  }
}
#endif //FULFILMONGOCPP_MONGO_MACHINE_H
