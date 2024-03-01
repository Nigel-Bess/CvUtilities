//
// Created by amber on 7/28/20.
//

#include "FulfilMongoCpp/mongo_parse/mongo_bsonxx_parser.h"
using ff_mongo_cpp::mongo_parse::MongoBsonxxParser;

#include "FulfilMongoCpp/mongo_objects/mongo_element.h"
using ff_mongo_cpp::mongo_objects::MongoElement;

#include "FulfilMongoCpp/mongo_objects/mongo_object_id.h"
using ff_mongo_cpp::mongo_objects::MongoObjectID;

#include "FulfilMongoCpp/mongo_objects/mongo_element.h"
using ff_mongo_cpp::mongo_objects::MongoElement;

MongoBsonxxParser::MongoBsonxxParser(bsoncxx::document::view doc)
{
  this->document = doc;
  this->current_element = nullptr;
}

MongoBsonxxParser::MongoBsonxxParser()
{
  this->document = bsoncxx::document::view();
  this->current_element = nullptr;
}

bool MongoBsonxxParser::cursorAtRoot()
{
  return (this->current_element == nullptr);
}

void MongoBsonxxParser::reset()
{
  this->current_element = nullptr;
}
