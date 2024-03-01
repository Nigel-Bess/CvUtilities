//
// Created by amber on 7/27/20.
//


#include "FulfilMongoCpp/mongo_objects/mongo_element.h"
using ff_mongo_cpp::mongo_objects::MongoElement;

#include "FulfilMongoCpp/mongo_objects/mongo_object_id.h"
using ff_mongo_cpp::mongo_objects::MongoObjectID;


bool MongoElement::tryGetElem(const bsoncxx::document::element& element, int& container)
{
  if(element && element.type() == bsoncxx::type::k_int32) {
  container = element.get_int32().value;
  return 1;
}
return 0;
}
bool MongoElement::tryGetElem(const bsoncxx::document::element& element, double& container)
{
  if(element && element.type() == bsoncxx::type::k_double) {
  container = element.get_double().value;
  return 1;
}
return 0;
}

bool MongoElement::tryGetElem(const bsoncxx::document::element& element, bool& container)
{
  if(element && element.type() == bsoncxx::type::k_bool) {
    container = element.get_bool().value;
    return 1;
  }
  return 0;

}

// Gets from array elements
// TODO figure out how to fold these (doc and array vals) into a single type
bool MongoElement::tryGetElem(const bsoncxx::document::element& element, std::string& container)
{
  if(element && element.type() == bsoncxx::type::k_utf8) {
    container = element.get_utf8().value.to_string();
    return 1;
  }
  return 0;
}

bool MongoElement::tryGetElem(const bsoncxx::document::element& element, MongoObjectID& container)
{
  if(element && element.type() == bsoncxx::type::k_oid) {
    container = MongoObjectID(element.get_oid().value);
    return 1;
  } else if (element && element.type() == bsoncxx::type::k_document){
    auto new_doc = element.get_document().view();
    bsoncxx::document::element element2 = new_doc["$oid"];
    container = MongoObjectID(element2.get_oid().value);
    return 1;
  }
  return 0;
}

bool MongoElement::tryGetElem(const bsoncxx::array::element& element, int& container)
{
  if(element && element.type() == bsoncxx::type::k_int32) {
  container = element.get_int32().value;
  return 1;
}
return 0;
}
bool MongoElement::tryGetElem(const bsoncxx::array::element& element, double& container)
{
  if(element && element.type() == bsoncxx::type::k_double) {
  container = element.get_double().value;
  return 1;
}
return 0;
}

bool MongoElement::tryGetElem(const bsoncxx::array::element& element, std::string& container)
{
  if(element && element.type() == bsoncxx::type::k_utf8) {
    container = element.get_utf8().value.to_string();
    return 1;
  }
  return 0;
}

bool MongoElement::tryGetElem(const bsoncxx::array::element& element, bool& container)
{
  if(element && element.type() == bsoncxx::type::k_utf8) {
    container = element.get_bool().value;
    return 1;
  }
  return 0;
}

bool MongoElement::tryGetElem(const bsoncxx::array::element& element, MongoObjectID& container)
{
  if(element && element.type() == bsoncxx::type::k_oid) {
    container = MongoObjectID(element.get_oid().value);
    return 1;
  } else if (element && element.type() == bsoncxx::type::k_document){
    auto new_doc = element.get_document().view();
    bsoncxx::document::element element2 = new_doc["$oid"];
    container = MongoObjectID(element2.get_oid().value);
    return 1;
  }
  return 0;
}
