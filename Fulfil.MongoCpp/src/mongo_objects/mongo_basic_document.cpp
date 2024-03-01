//
// Created by amber on 7/29/20.
//
#include "FulfilMongoCpp/mongo_objects/mongo_basic_document.h"
using ff_mongo_cpp::mongo_objects::MongoBasicDocument;
using ff_mongo_cpp::mongo_parse::MongoBsonxxParser;
using ff_mongo_cpp::mongo_parse::MongoBsonxxEncoder;
using ff_mongo_cpp::mongo_objects::MongoDocument;
using ff_mongo_cpp::mongo_objects::MongoGenericDocument;

MongoBasicDocument::MongoBasicDocument(bsoncxx::document::view doc_view,
                                     const std::string& collection_name, const std::string& db_name)
{
  if (doc_view.find("_id") == doc_view.end()) {
    this->oid = MongoObjectID();
    this->document_data = std::make_shared<bsoncxx::document::value>(doc_view);
  } else {
    this->oid = MongoObjectID(doc_view["_id"].get_oid().value);
    parse_in_values(doc_view);
  }
  this->parser = MongoBsonxxParser(this->document_data->view());
  this->collection_name = collection_name;
  this->db_name = db_name;

}

/**
 * Avoid using this constructor, ideally use the view constructor since it minimizes the
 * number of copies made.
 * */
MongoBasicDocument::MongoBasicDocument(bsoncxx::document::value doc,
                                     const std::string& collection_name, const std::string& db_name)
{

  bsoncxx::document::view doc_view = doc.view();
  if (doc_view.find("_id") == doc_view.end()) {
    this->oid = MongoObjectID();
    this->document_data = std::make_shared<bsoncxx::document::value>(doc);
  } else {
    this->oid = MongoObjectID(doc_view["_id"].get_oid().value);
    parse_in_values(doc_view);
  }
  this->parser = MongoBsonxxParser(this->document_data->view());
  this->collection_name = collection_name;
  this->db_name = db_name;
}

MongoBasicDocument::MongoBasicDocument(MongoBsonxxEncoder& doc_encoder,
                                       const std::string& collection_name, const std::string& db_name)
{
    bsoncxx::document::value doc = doc_encoder.extract();
    bsoncxx::document::view doc_view = doc.view();
    if (doc_view.find("_id") == doc_view.end()) {
        this->oid = MongoObjectID();
        this->document_data = std::make_shared<bsoncxx::document::value>(doc);
    } else {
        this->oid = MongoObjectID(doc_view["_id"].get_oid().value);
        parse_in_values(doc_view);
    }
    this->parser = MongoBsonxxParser(this->document_data->view());
    this->collection_name = collection_name;
    this->db_name = db_name;
}

MongoBasicDocument::MongoBasicDocument(){
  this->oid = MongoObjectID();
  this->collection_name = "";
  this->db_name = "";
  this->document_data = std::make_shared<bsoncxx::document::value>(bsoncxx::builder::basic::document{}.extract());
  this->parser = MongoBsonxxParser();
}

int MongoBasicDocument::parse_in_values(bsoncxx::document::view doc)
{
  using bsoncxx::builder::basic::kvp;
  bsoncxx::builder::basic::document doc_data{};
  bsoncxx::stdx::string_view _id = "_id";
  for (bsoncxx::document::element elem : doc) {
    if (elem.key() != _id) {
      doc_data.append(kvp(elem.key(), elem.get_value()));
    }
  }
  this->document_data = std::make_shared<bsoncxx::document::value>(doc_data.extract());

  return 0;
}

void MongoBasicDocument::SetObjectID(MongoObjectID new_id)
{
  this->oid = new_id;
}

ff_mongo_cpp::mongo_objects::MongoObjectID MongoBasicDocument::GetObjectID()
{
  return this->oid;
}

std::string MongoBasicDocument::GetCollection()
{
  return this->collection_name;
}

std::string MongoBasicDocument::GetDataBase()
{
  return this->db_name;
}

void MongoBasicDocument::SetCollection(const std::string& new_collection_name)
{
  this->collection_name = new_collection_name;
}

void MongoBasicDocument::SetDataBase(const std::string& new_db_name)
{
  this->db_name = new_db_name;
}

bsoncxx::document::value MongoBasicDocument::MakeWritableValue()
{
  auto doc_copy = bsoncxx::builder::stream::document{}
  << bsoncxx::builder::concatenate_doc{ this->document_data->view() }
  << bsoncxx::builder::stream::finalize;
  return doc_copy;
}

bool MongoBasicDocument::empty() const
{
  return (this->document_data->view()).empty();
}

std::string MongoBasicDocument::str() const
{
  std::stringstream doc_str;
  doc_str << bsoncxx::to_json(this->document_data->view());
  return doc_str.str();
}


std::shared_ptr<MongoDocument> MongoBasicDocument::MakeNewMongoDocument(bsoncxx::document::view doc,
                                                            const std::string& collection_name, const std::string& db_name)
{
  std::shared_ptr<MongoBasicDocument> new_doc (new  MongoBasicDocument(doc, collection_name, db_name));
  return new_doc;
}

std::shared_ptr<MongoGenericDocument> MongoBasicDocument::MakeNewGenericDocument(bsoncxx::document::view doc,
                                                                          const std::string& collection_name, const std::string& db_name)
{
    std::shared_ptr<MongoBasicDocument> new_doc (new  MongoBasicDocument(doc, collection_name, db_name));
    return new_doc;
}

void MongoBasicDocument::update_values(bsoncxx::document::view doc) {
  parse_in_values(doc);
}
