//
// Created by amber on 7/27/20.
//
#include "FulfilMongoCpp/mongo_json/mongo_json_document.h"

using ff_mongo_cpp::mongo_objects::MongoJsonDocument;
using ff_mongo_cpp::mongo_objects::MongoDocument;
using ff_mongo_cpp::mongo_objects::MongoGenericDocument;

MongoJsonDocument::MongoJsonDocument(bsoncxx::document::view doc,
                                     const std::string& collection_name, const std::string& db_name)
{
  if (doc.find("_id") == doc.end()){
    this->oid = MongoObjectID();
    this->parse_state = this->parse_in_values(doc);
  } else {
    //&& doc.view()["_id"].type() != bsoncxx::type::k_oid
    this->parse_state = this->parse_in_values(doc);
    if (doc["_id"].type() == bsoncxx::type::k_oid)
      this->oid = MongoObjectID(this->data["_id"]["$oid"].get<std::string>());
    else this->oid = MongoObjectID(this->data["_id"].get<std::string>());
    this->data.erase("_id");
  }
  this->collection_name = collection_name;
  this->db_name = db_name;

}


MongoJsonDocument::MongoJsonDocument(bsoncxx::document::value doc,
                                     const std::string& collection_name, const std::string& db_name)
{
  if (doc.view().find("_id") == doc.view().end()){
    this->oid = MongoObjectID();
    this->parse_state = this->parse_in_values(doc.view());
  } else {
    this->parse_state = parse_in_values(doc.view());
    if (doc.view()["_id"].type() == bsoncxx::type::k_oid)
      this->oid = MongoObjectID(this->data["_id"]["$oid"].get<std::string>());
    else this->oid = MongoObjectID(this->data["_id"].get<std::string>());
    this->data.erase("_id");
  }
  this->collection_name = collection_name;
  this->db_name = db_name;
}

MongoJsonDocument::MongoJsonDocument(nlohmann::json doc, const std::string& collection_name, const std::string& db_name)
{
  this->data = doc;
  if (this->data.find("_id") == this->data.end()){
    this->oid = MongoObjectID();
  } else {
    auto id = doc["_id"];
    if (id.find("$oid") != id.end())
      this->oid = MongoObjectID(this->data["_id"]["$oid"].get<std::string>());
    else this->oid = MongoObjectID(this->data["_id"].get<std::string>());
    this->data.erase("_id");
  }
  this->collection_name = collection_name;
  this->db_name = db_name;
}

MongoJsonDocument::MongoJsonDocument(const std::string& doc, const std::string& collection_name, const std::string& db_name)
{
  this->data = nlohmann::json::parse(doc);
  if (this->data.find("_id") == this->data.end()){
    this->oid = MongoObjectID();
  } else {
    auto id = this->data["_id"];
    if (id.find("$oid") != id.end())
      this->oid = MongoObjectID(this->data["_id"]["$oid"].get<std::string>());
    else this->oid = MongoObjectID(this->data["_id"].get<std::string>());
    this->data.erase("_id");
  }
  this->collection_name = collection_name;
  this->db_name = db_name;
}

MongoJsonDocument::MongoJsonDocument(){
  this->oid = MongoObjectID();
  this->parse_state = -1;
  this->collection_name = "";
  this->db_name = "";
}

ff_mongo_cpp::mongo_objects::MongoObjectID MongoJsonDocument::GetObjectID()
{
  return this->oid;
}

std::string MongoJsonDocument::GetCollection()
{
  return this->collection_name;
}

std::string MongoJsonDocument::GetDataBase()
{
  return this->db_name;
}

void MongoJsonDocument::SetCollection(const std::string& new_collection_name)
{
  this->collection_name = new_collection_name;
}

void MongoJsonDocument::SetDataBase(const std::string& new_db_name)
{
  this->db_name = new_db_name;
}


bsoncxx::document::value MongoJsonDocument::MakeWritableValue()
{
  return bsoncxx::document::value(bsoncxx::from_json(this->data.dump()));;
}



int MongoJsonDocument::parse_in_values(bsoncxx::document::view doc)
{
  std::stringstream doc_str;
  doc_str << bsoncxx::to_json(doc);
  this->data = nlohmann::json::parse(doc_str.str());
  return 0;
}

bool MongoJsonDocument::empty() const
{
  return (this->data.size() == 0);
}

std::string MongoJsonDocument::str() const
{
  return this->data.dump();
}

std::shared_ptr<MongoDocument>MongoJsonDocument::MakeNewMongoDocument(bsoncxx::document::view doc,
                                                                          const std::string& collection_name, const std::string& db_name)
{
    std::shared_ptr<MongoJsonDocument> new_doc (new  MongoJsonDocument(doc, collection_name, db_name));
    return new_doc;
}

std::shared_ptr<MongoGenericDocument> MongoJsonDocument::MakeNewGenericDocument(bsoncxx::document::view doc,
                                                                                    const std::string& collection_name, const std::string& db_name)
{
    std::shared_ptr<MongoJsonDocument> new_doc (new  MongoJsonDocument(doc, collection_name, db_name));
    return new_doc;
}


void ff_mongo_cpp::mongo_objects::MongoJsonDocument::SetObjectID(MongoObjectID new_id) {
  this->oid = new_id;
}

void MongoJsonDocument::update_values(bsoncxx::document::view doc) {
  if (doc.find("_id") == doc.end()){
    this->oid = MongoObjectID();
    this->parse_state = this->parse_in_values(doc);
  } else {
    this->parse_state = this->parse_in_values(doc);
    if (doc["_id"].type() == bsoncxx::type::k_oid)
      this->oid = MongoObjectID(this->data["_id"]["$oid"].get<std::string>());
    else this->oid = MongoObjectID(this->data["_id"].get<std::string>());
    this->data.erase("_id");
  }

}
