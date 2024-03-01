//
// Created by amber on 8/12/20.
//
#include "FulfilMongoCpp/mongo_parse/mongo_bsonxx_encoder.h"

using ff_mongo_cpp::mongo_parse::MongoBsonxxEncoder;


void MongoBsonxxEncoder::concatenate(bsoncxx::document::view value){
  this->master_doc_builder.append( bsoncxx::builder::concatenate_doc{ value } );
}

bsoncxx::document::value MongoBsonxxEncoder::extract(){
  bsoncxx::document::value master_doc_value = this->master_doc_builder.extract();
  this->master_doc_builder = bsoncxx::builder::basic::document{};
  return master_doc_value;
}

void MongoBsonxxEncoder::addCurrentTimeField(std::string key)
{
    using bsoncxx::builder::basic::kvp;
    master_doc_builder.append(kvp(key,
        bsoncxx::types::b_date{std::chrono::system_clock::now()}));
}

bsoncxx::types::b_oid MongoBsonxxEncoder::convert_bson(ff_mongo_cpp::mongo_objects::MongoObjectID value)
{
  return bsoncxx::types::b_oid{bsoncxx::oid(value)};
}

bsoncxx::types::b_array MongoBsonxxEncoder::convert_bson(bsoncxx::array::view value)
{
  return bsoncxx::types::b_array{ value };
}

bsoncxx::types::b_document MongoBsonxxEncoder::convert_bson(bsoncxx::document::view value)
{
  return bsoncxx::types::b_document{ value };
}

bsoncxx::types::b_document MongoBsonxxEncoder::convert_bson(MongoFilter value)
{
  return bsoncxx::types::b_document{ value.view() };
}