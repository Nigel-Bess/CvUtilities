//
// Created by amber on 8/14/20.
//

//
// Created by amber on 7/26/20.
//
#include "FulfilMongoCpp/mongo_connection.h"
using ff_mongo_cpp::MongoConnection;
#include "FulfilMongoCpp/base_mongo_connection.h"
using ff_mongo_cpp::BaseMongoConnection;
#include "FulfilMongoCpp/mongo_objects/mongo_object_id.h"
using ff_mongo_cpp::mongo_objects::MongoObjectID;
#include "FulfilMongoCpp/mongo_filter/mongo_filters.h"
using ff_mongo_cpp::mongo_filter::MongoFilter;
#include "FulfilMongoCpp/mongo_objects/mongo_document.h"
using ff_mongo_cpp::mongo_objects::MongoDocument;



MongoConnection::MongoConnection(const std::string& conn_string)
  : BaseMongoConnection(conn_string) {}


// Find functions
bool MongoConnection::tryFindAll(const std::string &DBName, const std::string &collection_name,
                                 std::shared_ptr<std::vector<std::shared_ptr<MongoDocument>>> docs,
                                 std::shared_ptr<MongoDocument> template_doc)
{
  try {
    mongocxx::cursor cursor = BaseMongoConnection::findAll(DBName, collection_name);
    BaseMongoConnection::CursorToMongoDocument(cursor, collection_name, DBName, docs, template_doc);
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

// ObjectId based finds
bool MongoConnection::tryFindByObjectIDs(const std::string &DBName, const std::string &collection_name,
        const std::vector<bsoncxx::oid> &oids, std::shared_ptr<std::vector<std::shared_ptr<MongoDocument>>> docs,
                                         std::shared_ptr<MongoDocument> template_doc)
{
  try {
    mongocxx::cursor cursor = BaseMongoConnection::findManyByObjectIDs(DBName, collection_name,oids);
    BaseMongoConnection::CursorToMongoDocument(cursor, collection_name, DBName, docs, template_doc);
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}
bool MongoConnection::tryFindByObjectIDs(const std::string &DBName, const std::string &collection_name,
                                         const std::vector<MongoObjectID> &oids, std::shared_ptr<std::vector<std::shared_ptr<MongoDocument>>> docs,
                                         std::shared_ptr<MongoDocument> template_doc) {
  return tryFindByObjectIDs(DBName, collection_name, MongoObjectID::convertToBSonIdVec(oids), docs, template_doc);
}

bool MongoConnection::tryFindByObjectIDs(const std::string &DBName, const std::string &collection_name,
                                         const std::vector<std::string> &oids, std::shared_ptr<std::vector<std::shared_ptr<MongoDocument>>> docs,
                                         std::shared_ptr<MongoDocument> template_doc) {
  return tryFindByObjectIDs(DBName, collection_name, MongoObjectID::convertToBSonIdVec(oids), docs, template_doc);
}

bool MongoConnection::tryFindOneByObjectID(const std::string &DBName, const std::string &collection_name,
                                   MongoObjectID oid,  std::shared_ptr<MongoDocument> doc)
{

  bsoncxx::stdx::optional<bsoncxx::document::value> found_doc = BaseMongoConnection::findOneByObjectID(DBName,
          collection_name, bsoncxx::oid(oid));
  if (found_doc) {
    doc->update_values(found_doc->view());
    return true;
  }
  return false;
}

bool MongoConnection::tryFindOneByFilter(const std::string &DBName, const std::string &collection_name,
                                         mongo_filter::MongoFilter filter,  std::shared_ptr<MongoDocument> doc)
{

  bsoncxx::stdx::optional<bsoncxx::document::value> found_doc = BaseMongoConnection::findOneByFilter(DBName,
                                                                                   collection_name, filter.view());
  if (found_doc) {
    doc->update_values(found_doc->view());
    return true;
  }
  return false;
}

// Filter finds
bool MongoConnection::tryFindByFilter(const std::string &DBName, const std::string &collection_name,
        mongo_filter::MongoFilter filter, std::shared_ptr<std::vector<std::shared_ptr<MongoDocument>>> docs,
        std::shared_ptr<MongoDocument> template_doc)
{
  try {
    mongocxx::cursor cursor = BaseMongoConnection::findManyByFilter(DBName, collection_name, filter.view());
    BaseMongoConnection::CursorToMongoDocument(cursor, collection_name, DBName, docs, template_doc);
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

bool MongoConnection::tryFindByFilter(const std::string &DBName, const std::string &collection_name,
                                      mongo_filter::MongoFilter filter, std::shared_ptr<std::vector<std::shared_ptr<MongoGenericDocument>>> docs,
                                      std::shared_ptr<MongoGenericDocument> template_doc)
{
  try {
    mongocxx::cursor cursor = BaseMongoConnection::findManyByFilter(DBName, collection_name, filter.view());
    BaseMongoConnection::CursorToMongoDocument(cursor, collection_name, DBName, docs, template_doc);
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}


// Insertions
bool MongoConnection::tryInsertOne(const std::string &DBName, const std::string &collection_name,
                                   std::shared_ptr<MongoDocument> doc,
                                   MongoObjectID &insert_id)
{
  bsoncxx::document::value doc_value = doc->MakeWritableValue();
  bsoncxx::oid bson_oid;
  bool success = BaseMongoConnection::tryInsertOne(DBName, collection_name, doc_value.view(), bson_oid);
  insert_id = MongoObjectID(bson_oid);
  return success;
}

bool MongoConnection::tryInsertOne(std::shared_ptr<MongoDocument> doc, MongoObjectID &insert_id)
{
  return tryInsertOne(doc->GetDataBase(), doc->GetCollection(), doc, insert_id);
}


int MongoConnection::tryInsertMany(const std::string &DBName, const std::string &collection_name,
                                   std::shared_ptr<std::vector<std::shared_ptr<MongoDocument>>> inserts,
                                   std::vector<MongoObjectID> &insert_ids)
{
  std::shared_ptr<std::vector<bsoncxx::document::value>> bson_values = std::make_shared<std::vector<bsoncxx::document::value>>();
  for (auto doc : *inserts) {
    bsoncxx::document::value doc_value = doc->MakeWritableValue();
    bson_values->emplace_back(doc_value);
  }
  std::vector<bsoncxx::oid> bson_oids;
  int success = BaseMongoConnection::tryInsertMany(DBName, collection_name, bson_values, bson_oids);
  insert_ids = MongoObjectID::convertToMongoIdVec(bson_oids);
  return success;
}

int MongoConnection::tryInsertMany(std::shared_ptr<std::vector<std::shared_ptr<MongoDocument>>> inserts,
                  std::vector<MongoObjectID> &insert_ids)
{

  for (auto doc : *inserts) {
    MongoObjectID insert_id;
    if (tryInsertOne(doc, insert_id))
      insert_ids.emplace_back(insert_id);
  }
  return insert_ids.size();
}

// Updates
bool MongoConnection::tryUpdateOneByFilter(const std::string &DBName, const std::string &collection_name,
                                           mongo_filter::MongoFilter filter,
                                           std::shared_ptr<MongoDocument> doc, const std::string &update_type)
{

  bsoncxx::document::value doc_value = doc->MakeWritableValue();
  bool success = BaseMongoConnection::tryUpdateOneByFilter(DBName, collection_name, filter.view(), doc_value.view(),
                                                           update_type);
  return success;
}

bool MongoConnection::tryUpdateOneByFilter(mongo_filter::MongoFilter filter,
                                           std::shared_ptr<MongoDocument> doc, const std::string &update_type)
{
  return tryUpdateOneByFilter(doc->GetDataBase(), doc->GetCollection(), filter, doc, update_type);
}

int MongoConnection::tryUpdateManyByFilter(const std::string &DBName, const std::string &collection_name,
                                           mongo_filter::MongoFilter filter,
                                           std::shared_ptr<MongoDocument> doc, const std::string &update_type)
{
  bsoncxx::document::value doc_value = doc->MakeWritableValue();
  int success = BaseMongoConnection::tryUpdateManyByFilter(DBName, collection_name, filter.view(), doc_value.view(),
                                                           update_type);
  return success;

}

bool MongoConnection::tryUpdateOneByObjectID(const std::string &DBName, const std::string &collection_name,
                                             std::shared_ptr<MongoDocument> doc, MongoObjectID oid, const std::string &update_type)
{
  bsoncxx::document::value doc_value = doc->MakeWritableValue();
  bool success = BaseMongoConnection::tryUpdateOneByObjectID(DBName, collection_name, doc_value.view(),
                                                             bsoncxx::oid(oid), update_type);
  return success;
}

bool MongoConnection::tryUpdateOneByObjectID(std::shared_ptr<MongoDocument> doc, MongoObjectID oid,
        const std::string &update_type)
{
  return tryUpdateOneByObjectID(doc->GetDataBase(), doc->GetCollection(), doc, oid, update_type);
}

int MongoConnection::tryUpdateManyByObjectIDs(const std::string &DBName, const std::string &collection_name,
            std::shared_ptr<MongoDocument> doc, const std::vector<bsoncxx::oid> &oids, const std::string &update_type)
{
  bsoncxx::document::value doc_value = doc->MakeWritableValue();
  int success = BaseMongoConnection::tryUpdateManyByObjectIDs(DBName, collection_name, doc_value.view(), oids,
                                                              update_type);
  return success;
}

// Deletes
bool MongoConnection::tryDeleteOneByFilter(const std::string &DBName, const std::string &collection_name,
                                           mongo_filter::MongoFilter filter)
{


  bool success = BaseMongoConnection::tryDeleteOneByFilter(DBName, collection_name, filter.view());
  return success;
}

int MongoConnection::tryDeleteManyByFilter(const std::string &DBName, const std::string &collection_name,
                                           mongo_filter::MongoFilter filter)
{
  int success = BaseMongoConnection::tryDeleteManyByFilter(DBName, collection_name, filter.view());
  return success;
}

bool MongoConnection::tryDeleteOneByObjectID(const std::string &DBName, const std::string &collection_name, MongoObjectID oid)
{
  bool success = BaseMongoConnection::tryDeleteOneByObjectID(DBName, collection_name, bsoncxx::oid(oid));
  return success;
}

int MongoConnection::tryDeleteManyByObjectIDs(const std::string &DBName, const std::string &collection_name,
                             const std::vector<std::string> &oids)
{
  int success = BaseMongoConnection::tryDeleteManyByObjectIDs(DBName, collection_name,
                                                              MongoObjectID::convertToBSonIdVec(oids));
  return success;
}

int MongoConnection::tryDeleteManyByObjectIDs(const std::string &DBName, const std::string &collection_name,
                                              const std::vector<MongoObjectID> &oids)
{
  int success = BaseMongoConnection::tryDeleteManyByObjectIDs(DBName, collection_name,
                                                              MongoObjectID::convertToBSonIdVec(oids));
  return success;
}

