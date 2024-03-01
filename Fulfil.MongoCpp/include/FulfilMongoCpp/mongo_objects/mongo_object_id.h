//
// Created by amber on 7/26/20.
//

#ifndef FULFIL_DISPENSE_MONGO_OBJECT_ID_H
#define FULFIL_DISPENSE_MONGO_OBJECT_ID_H
#include <bsoncxx/oid.hpp>
#include <bsoncxx/exception/exception.hpp>
#include <iomanip>
#include <vector>
#include <ctime>
#include <sstream>


namespace ff_mongo_cpp {
    namespace mongo_objects {
        class MongoObjectID {
        public:
            MongoObjectID() = default;

            MongoObjectID(const std::string &oid);

            MongoObjectID(const bsoncxx::oid &oid);

            MongoObjectID(int year, int month, int day,
                    int hour=0, int min=0, int sec=0,
                    bool use_local=true, const std::string& identifier="0000000000000000");

            static std::vector<bsoncxx::oid> convertToBSonIdVec(const std::vector<std::string> &oids);

            static std::vector<bsoncxx::oid> convertToBSonIdVec(const std::vector<MongoObjectID> &oids);

            static std::vector<MongoObjectID> convertToMongoIdVec(const std::vector<bsoncxx::oid> &oids);

            static std::vector<MongoObjectID> convertToMongoIdVec(const std::vector<std::string> &oids);

            void updateID();

            [[nodiscard]] std::time_t get_time_t() const;

            [[nodiscard]] bool is_null() const;

            [[nodiscard]] std::string get_time(bool use_local = true) const;

            operator bsoncxx::oid() const {
              return this->oid;
            }
            [[nodiscard]] std::string str() const {
              return this->oid.to_string();
            }
            [[nodiscard]] bsoncxx::oid bson_oid() const {
              return this->oid;
            }
            operator std::string() const {
              return this->oid.to_string();
            }

        private:
            bsoncxx::oid oid {"000000000000000000000000"};
            static std::string get_hextime(int year, int month, int day, int hour=0, int min=0, int sec=0, bool use_local=true);


            // Can be any type with an bsoncxx() initializer or operator
            template<typename T, typename U>
            static std::vector<T> convertIdVec(const std::vector<U> &oids)
            {
              std::vector<T> converted{};
              converted.reserve(oids.size());
              for (auto & oid : oids)
                converted.emplace_back(oid);
              return converted;
            }

        };
    }
}
#endif //FULFIL_DISPENSE_MONGO_OBJECT_ID_H
