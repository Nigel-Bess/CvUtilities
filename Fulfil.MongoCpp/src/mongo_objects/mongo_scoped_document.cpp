//
// Created by amber on 7/29/20.
//

//
// Created by amber on 7/29/20.
//
#include "FulfilMongoCpp/mongo_objects/mongo_scoped_document.h"
using ff_mongo_cpp::mongo_objects::MongoScopedDocument;

using ff_mongo_cpp::mongo_objects::MongoDocument;
using ff_mongo_cpp::mongo_objects::MongoGenericDocument;


MongoScopedDocument::MongoScopedDocument(bsoncxx::document::view doc,
                                     const std::string& collection_name, const std::string& db_name)
{
  if (doc.find("_id") == doc.end()) this->oid = MongoObjectID();
  else this->oid = MongoObjectID(doc["_id"].get_oid().value);
  this->document_data = doc;
  this->collection_name = collection_name;
  this->db_name = db_name;
  this->parser = MongoBsonxxParser(this->document_data);
}


MongoScopedDocument::MongoScopedDocument(bsoncxx::document::value doc,
                                       const std::string& collection_name, const std::string& db_name)
{
  if (doc.view().find("_id") == doc.view().end()) this->oid = MongoObjectID();
  else this->oid = MongoObjectID(doc.view()["_id"].get_oid().value);
  this->document_data = doc.view();
  this->parser = MongoBsonxxParser(this->document_data);
  this->collection_name = collection_name;
  this->db_name = db_name;
}

MongoScopedDocument::MongoScopedDocument(){
  this->oid = MongoObjectID();
  this->collection_name = "";
  this->db_name = "";
  this->document_data = bsoncxx::document::view();
  this->parser = MongoBsonxxParser();
}

ff_mongo_cpp::mongo_objects::MongoObjectID MongoScopedDocument::GetObjectID()
{
  return this->oid;
}

std::string MongoScopedDocument::GetCollection()
{
  return this->collection_name;
}

std::string MongoScopedDocument::GetDataBase()
{
  return this->db_name;
}

void MongoScopedDocument::SetCollection(const std::string& new_collection_name)
{
  this->collection_name = new_collection_name;
}

void MongoScopedDocument::SetDataBase(const std::string& new_db_name)
{
  this->db_name = new_db_name;
}

bool MongoScopedDocument::empty() const
{
  return this->document_data.empty();
}

bsoncxx::document::value MongoScopedDocument::MakeWritableValue()
{
  using bsoncxx::builder::basic::kvp;
  bsoncxx::builder::basic::document doc_data{};
  bsoncxx::stdx::string_view _id = "_id";
  for (bsoncxx::document::element elem : this->document_data) {
    if (elem.key() != _id) {
      doc_data.append(kvp(elem.key(), elem.get_value()));
    }
  }
  return bsoncxx::document::value(this->document_data);
}

std::string MongoScopedDocument::str() const
{
  std::stringstream doc_str;
  doc_str << bsoncxx::to_json(this->document_data);
  return doc_str.str();
}

std::shared_ptr<MongoDocument>MongoScopedDocument::MakeNewMongoDocument(bsoncxx::document::view doc,
                                                                          const std::string& collection_name, const std::string& db_name)
{
    std::shared_ptr<MongoScopedDocument> new_doc (new  MongoScopedDocument(doc, collection_name, db_name));
    return new_doc;
}

std::shared_ptr<MongoGenericDocument> MongoScopedDocument::MakeNewGenericDocument(bsoncxx::document::view doc,
                                                                                   const std::string& collection_name, const std::string& db_name)
{
    std::shared_ptr<MongoScopedDocument> new_doc (new  MongoScopedDocument(doc, collection_name, db_name));
    return new_doc;
}

void ff_mongo_cpp::mongo_objects::MongoScopedDocument::SetObjectID(MongoObjectID new_id) {
  this->oid = new_id;
}

void MongoScopedDocument::update_values(bsoncxx::document::view doc) {
  if (doc.find("_id") != doc.end() && doc["_id"].type() != bsoncxx::type::k_oid) this->oid = MongoObjectID();
  this->document_data =doc;
  this->parser = MongoBsonxxParser(this->document_data);
}
