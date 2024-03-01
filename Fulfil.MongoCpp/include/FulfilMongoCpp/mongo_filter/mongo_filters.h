//
// Created by amber on 5/27/20.
//

#ifndef FULFIL_DISPENSE_MONGO_FILTERS_H_
#define FULFIL_DISPENSE_MONGO_FILTERS_H_

#include <iostream>
#include <sstream>
#include <typeinfo>
#include <vector>
#include <algorithm>

#include <memory>

#include "FulfilMongoCpp/mongo_objects/mongo_object_id.h"


#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <utility>
#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/types.hpp>


namespace ff_mongo_cpp {
    namespace mongo_filter {
        //TODO need to remove smart pointer and then resource manage
        class MongoFilter {
        public:
            enum class FilterType {
                Eq, Ne, Lt, Lte, Gt, Gte,
                In, Nin, Not
            };

            enum class CompositeType {
                And, Or
            };

            MongoFilter(std::string field_string, FilterType type, ff_mongo_cpp::mongo_objects::MongoObjectID field_value) {
              this->filter = makeFilter(field_string, type, bsoncxx::oid(field_value));
            }

            template<typename T>
            MongoFilter(std::string field_string, FilterType type, T field_value) {
              this->filter = makeFilter(field_string, type, field_value);
            }

            MongoFilter(bsoncxx::document::value filter_document) {
              this->filter = std::make_shared<bsoncxx::document::value>(filter_document);
            }

            MongoFilter(CompositeType composite_type, std::vector<MongoFilter> filters) {
              this->filter = makeCompositeFilter(composite_type, filters);
            }

            MongoFilter(std::string field_string, FilterType type, std::vector<ff_mongo_cpp::mongo_objects::MongoObjectID> field_value){
              this->filter = makeArrayFilter(field_string, type, ff_mongo_cpp::mongo_objects::MongoObjectID::convertToBSonIdVec(field_value));
            }

            template<typename T>
            MongoFilter(std::string field_string, FilterType type, std::vector<T> field_value) {
              this->filter = makeArrayFilter(field_string, type, field_value);
            }

            // TODO make private when done testing
            bsoncxx::document::value value();

            bsoncxx::document::view view();

            std::string str() const;
            //TODO make composite filter & struct for filter args (variadic on struct type)

            // overload string cast
            operator std::string() const {
              return str();
            }

            template<typename T>
            void appendToFilter(CompositeType composite_type, std::string field_string, FilterType type, T field_value)
            {
              std::vector<MongoFilter> filters = { MongoFilter(field_string, type, field_value) };
              appendToFilter(composite_type, filters);

            }

            template<typename T>
            void appendToFilter(CompositeType composite_type, std::string field_string, FilterType type, std::vector<T> field_value)
            {
              std::vector<MongoFilter> filters = { MongoFilter(field_string, type, field_value) };
              appendToFilter(composite_type, filters);
            }

            void appendToFilter(CompositeType composite_type, std::vector<MongoFilter> filters)
            {
              filters.push_back(*this);
              std::rotate(filters.rbegin(), filters.rbegin() + 1, filters.rend());
              this->filter = makeCompositeFilter(composite_type, filters);
            }

        private:
            static std::string FilterTypeToStr(FilterType type);
            static std::string CompositeTypeToStr(CompositeType type);
            std::shared_ptr<bsoncxx::document::value> makeCompositeFilter(CompositeType composite_type,
                                                                            std::vector<MongoFilter> filters);
            std::shared_ptr<bsoncxx::document::value> filter;



            template<typename T>
            std::shared_ptr<bsoncxx::document::value> makeFilter(const std::string &field, FilterType ft, T value) {
              auto filter_builder = bsoncxx::builder::stream::document{};
              bsoncxx::document::value _filter = filter_builder
                      << field << bsoncxx::builder::stream::open_document
                      << FilterTypeToStr(ft) << value
                      << bsoncxx::builder::stream::close_document
                      << bsoncxx::builder::stream::finalize;
              return std::make_shared<bsoncxx::document::value>(_filter);
            }

            /**
             * Array filters are good for checking composite type situations
             * */
            template<typename T>
            std::shared_ptr<bsoncxx::document::value> makeArrayFilter(std::string field, FilterType ft, std::vector<T> values) {
              using bsoncxx::builder::basic::sub_document;
              using bsoncxx::builder::basic::sub_array;
              using bsoncxx::builder::basic::kvp;
              auto filter_builder = bsoncxx::builder::basic::document{};
              filter_builder.append(kvp(field,
                                        [ft, values](sub_document filter_op) {
                                            filter_op.append(
                                                    kvp(FilterTypeToStr(ft), [values](sub_array filter_vals) {
                                                        for (auto &val : values) filter_vals.append(val);
                                                    }) //create sub array
                                            );//single sub array appended to subdoc
                                        }) // single subdoc appended
              );
              return std::make_shared<bsoncxx::document::value>(filter_builder.extract());
            }
        };
    }
}
#endif //FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_MONGO_CONN_MONGO_FILTERS_H_
