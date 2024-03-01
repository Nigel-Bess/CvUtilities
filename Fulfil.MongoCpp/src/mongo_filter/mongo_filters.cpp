//
// Created by amber on 5/27/20.
//



#include "FulfilMongoCpp/mongo_filter/mongo_filters.h"
using ff_mongo_cpp::mongo_filter::MongoFilter;


std::string MongoFilter::FilterTypeToStr(MongoFilter::FilterType type)
{
  std::string filterTypesStrings [] = {
          "$eq","$ne","$lt","$lte","$gt","$gte",
          "$in","$nin", "$not"
  };
  int type_convert = static_cast<std::underlying_type<MongoFilter::FilterType>::type>(type);
  return filterTypesStrings[type_convert];
}

std::string MongoFilter::CompositeTypeToStr(MongoFilter::CompositeType type)
{
  std::string compositeTypesStrings [] = {
          "$and","$or"
  };
  int type_convert = static_cast<std::underlying_type<MongoFilter::CompositeType>::type>(type);
  return compositeTypesStrings[type_convert];
}

bsoncxx::document::value MongoFilter::value(){
  return *this->filter;
}

bsoncxx::document::view MongoFilter::view(){
  return (*this->filter).view();
}



std::string MongoFilter::str() const
{
  std::stringstream filter_str;
  filter_str << bsoncxx::to_json((*this->filter).view());
  return filter_str.str();
}

std::shared_ptr<bsoncxx::document::value> MongoFilter::makeCompositeFilter(CompositeType composite_type,
                                                                std::vector<MongoFilter> filters){
    using bsoncxx::builder::basic::kvp;
    auto filter_arr = bsoncxx::builder::basic::array{};
    bsoncxx::builder::basic::document composite_filter{};
    for (auto&& f : filters) {
        filter_arr.append(bsoncxx::types::b_document{ f.view() } );
    }
    bsoncxx::array::value filter_arr_val = filter_arr.extract();
    composite_filter.append(kvp(CompositeTypeToStr(composite_type), bsoncxx::types::b_array{ filter_arr_val.view() }));
    return std::make_shared<bsoncxx::document::value>(composite_filter.extract());

}