//
// Created by amber on 7/26/20.
//

#include "FulfilMongoCpp/base_mongo_connection.h"
using ff_mongo_cpp::BaseMongoConnection;

#include "FulfilMongoCpp/mongo_objects/mongo_document.h"
using ff_mongo_cpp::mongo_objects::MongoDocument;

#include "FulfilMongoCpp/mongo_objects/mongo_generic_document.h"
using ff_mongo_cpp::mongo_objects::MongoGenericDocument;

BaseMongoConnection::BaseMongoConnection(const std::string& conn_string) {
  this->_connected = false;
  //TODO a constructor that takes instance args and ensures destruction
  mongocxx::options::client client_options;
  mongocxx::options::tls tls_options;
  tls_options.allow_invalid_certificates(true);
  client_options.tls_opts(tls_options);

  mongocxx::instance inst{};

  try {
    this->_client = mongocxx::client(mongocxx::uri{conn_string}, client_options);
    //this->_client = mongocxx::client(mongocxx::uri{conn_string});

    this->_connected = bool(this->_client);
    // Helps to verify connection
    this->avaliable_db = this->_client.list_database_names();
  } catch (mongocxx::exception& e) {
    std::cout << e.what() << std::endl;
    this->_connected = false;
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    this->_connected = false;
  }
}

bool BaseMongoConnection::tryGetCollection(const std::string& DBName, const std::string& collection_name,
                                           mongocxx::collection& collection)
{
  try{
    // TODO add some more specific exception handling
    // TODO will not actually return false if collection does not exist so long as connection valid

    //check if database exists
    std::vector<std::string> db_names = _client.list_database_names();
    if (std::find(db_names.begin(), db_names.end(), DBName) == db_names.end()) return false;

    //check if collection exists
    if (!_client[DBName].has_collection(collection_name)) return false;

    collection = this->_client[DBName][collection_name];
    return true;
  } catch (mongocxx::exception& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}

mongocxx::cursor BaseMongoConnection::findAll(const std::string& DBName, const std::string& collection_name)
{
  auto collection = this->_client[DBName][collection_name];
  mongocxx::cursor c = collection.find({});
  return c;
}

bsoncxx::stdx::optional<bsoncxx::document::value> BaseMongoConnection::findOneByObjectID(const std::string& DBName,
                                                                                         const std::string& collection_name, const bsoncxx::oid &oid)
{
  using bsoncxx::builder::basic::kvp;
  auto collection = mongocxx::collection();
  this->tryGetCollection(DBName, collection_name, collection);
  auto filter = bsoncxx::builder::basic::document{};
  filter.append(kvp("_id", oid));
  bsoncxx::stdx::optional<bsoncxx::document::value> res = collection.find_one({filter.extract().view()});
  return res;
}

bsoncxx::stdx::optional<bsoncxx::document::value> BaseMongoConnection::findOneByFilter(const std::string &DBName, const std::string &collection_name,
                                                       bsoncxx::document::view filter)
{
  auto collection = mongocxx::collection();
  this->tryGetCollection(DBName, collection_name, collection);
  bsoncxx::stdx::optional<bsoncxx::document::value> res = collection.find_one({filter});
  return res;
}

mongocxx::cursor BaseMongoConnection::findManyByObjectIDs(const std::string &DBName, const std::string &collection_name, const std::vector<bsoncxx::oid> &oids)
{
  auto collection = mongocxx::collection();
  this->tryGetCollection(DBName, collection_name, collection);
  return findManyByObjectIDs(collection, oids);

}

mongocxx::cursor BaseMongoConnection::findManyByObjectIDs(mongocxx::collection collection, const std::vector<bsoncxx::oid> &oids)
{
  using bsoncxx::builder::basic::kvp;
  auto filter = bsoncxx::builder::basic::document{};
  filter.append(kvp("_id", bsoncxx::types::b_document{ makeBsonObjectIDArrayQuery(oids).view() }));
  mongocxx::cursor c = collection.find({filter.extract().view()});
  return c;
}

mongocxx::cursor BaseMongoConnection::findManyByFilter(const std::string &DBName, const std::string &collection_name,
                                                       bsoncxx::document::view filter)
{
  auto collection = mongocxx::collection();
  if (!tryGetCollection(DBName, collection_name, collection))
    std::cerr << "Failed to get collection " << DBName << "." << collection_name << "!\n";
  mongocxx::cursor c = collection.find({filter});
  return c;
}

bool BaseMongoConnection::tryInsertOne(const std::string &DBName, const std::string &collection_name,
                                       bsoncxx::document::view doc,
                                       bsoncxx::oid &insert_id)
{
  mongocxx::collection collection;
  if (!tryGetCollection(DBName, collection_name, collection)){
      insert_id = bsoncxx::oid("000000000000000000000000");
      return false;
  }
  bsoncxx::stdx::optional<mongocxx::result::insert_one> result;
  try {
    result = collection.insert_one(doc);
    if (!result) {
      insert_id = bsoncxx::oid("000000000000000000000000");
      return false;
    }
    insert_id = result->inserted_id().get_oid().value;
  } catch (mongocxx::exception& e) {
    std::cerr <<  "Insert error for DB "<< DBName << " Collection " << collection_name << ":\n\t" << e.what() << std::endl;
    insert_id = bsoncxx::oid("000000000000000000000000");
    return false;
  }
  return true;
}

int BaseMongoConnection::tryInsertMany(const std::string &DBName, const std::string &collection_name,
                                       std::shared_ptr<std::vector<bsoncxx::document::value>> inserts,
                                       std::vector<bsoncxx::oid>& insert_ids)
{
  mongocxx::collection collection;
  if (!tryGetCollection(DBName, collection_name, collection)) return -1;
  bsoncxx::stdx::optional<mongocxx::result::insert_many> result;
  try {
    result = collection.insert_many(*inserts);
    if (!result) return 0;
    for (auto id : result->inserted_ids()) insert_ids.emplace_back(id.second.get_oid().value);
  } catch (mongocxx::exception& e) {
    std::cout << e.what() << std::endl;
    return 0;
  }
  if(result) return result->inserted_count();
  return 0;
}

bool BaseMongoConnection::tryUpdateOneByFilter(const std::string &DBName, const std::string &collection_name,
                                               bsoncxx::document::view filter,
                                               bsoncxx::document::view doc, const std::string &update_type)
{
  mongocxx::collection collection;
  if (!tryGetCollection(DBName, collection_name, collection)) return false;
  bsoncxx::stdx::optional<mongocxx::result::update> result;
  try {
    result = collection.update_one(bsoncxx::builder::stream::document{}
                                           << bsoncxx::builder::concatenate_doc{ filter } << bsoncxx::builder::stream::finalize, // pulls mongo ref by id
                                   bsoncxx::builder::stream::document{} << update_type << bsoncxx::builder::stream::open_document
                                                                        << bsoncxx::builder::concatenate_doc{ doc } // loads in the values contained in the document to update
                                                                        << bsoncxx::builder::stream::close_document
                                                                        << bsoncxx::builder::stream::finalize);
    if(result->matched_count() < 1) return false;
  } catch (mongocxx::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

int BaseMongoConnection::tryUpdateManyByFilter(const std::string &DBName, const std::string &collection_name,
                                               bsoncxx::document::view filter,
                                               bsoncxx::document::view doc,
                                               const std::string &update_type)
{
  mongocxx::collection collection;
  if (!tryGetCollection(DBName, collection_name, collection)) return -1;
  bsoncxx::stdx::optional<mongocxx::result::update> result;
  try {
    result = collection.update_many(bsoncxx::builder::stream::document{}
                                            << bsoncxx::builder::concatenate_doc{ filter } << bsoncxx::builder::stream::finalize, // pulls mongo ref by id
                                    bsoncxx::builder::stream::document{} << update_type << bsoncxx::builder::stream::open_document
                                                                         << bsoncxx::builder::concatenate_doc{ doc } // loads in the values contained in the document to update
                                                                         << bsoncxx::builder::stream::close_document
                                                                         << bsoncxx::builder::stream::finalize);
  } catch (mongocxx::exception& e) {
    std::cout << e.what() << std::endl;
    return 0;
  }
  if(result) return result->modified_count();

  return 0;
}

int BaseMongoConnection::tryUpdateManyByObjectIDs(const std::string &DBName, const std::string &collection_name,
                                                  bsoncxx::document::view doc,
                                                  const std::vector<bsoncxx::oid> &oids, const std::string &update_type){
  using bsoncxx::builder::basic::kvp;
  auto filter = bsoncxx::builder::basic::document{};
  filter.append(kvp("_id", bsoncxx::types::b_document{ makeBsonObjectIDArrayQuery(oids).view() }));
  return tryUpdateManyByFilter(DBName, collection_name, filter, doc, update_type);
}


bool BaseMongoConnection::tryUpdateOneByObjectID(const std::string &DBName, const std::string &collection_name,
                                                 bsoncxx::document::view doc,
                                                 bsoncxx::oid oid, const std::string &update_type){
  using bsoncxx::builder::basic::kvp;
  auto filter = bsoncxx::builder::basic::document{};
  filter.append(kvp("_id", oid));
  return tryUpdateOneByFilter(DBName, collection_name, filter, doc, update_type);
}

bool BaseMongoConnection::tryDeleteOneByFilter(const std::string &DBName, const std::string &collection_name, bsoncxx::document::view filter)
{
  mongocxx::collection collection;
  if (!tryGetCollection(DBName, collection_name, collection)) return false;
  bsoncxx::stdx::optional<mongocxx::result::delete_result> result;
  try {
    result = collection.delete_one({filter});
  } catch (mongocxx::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

int BaseMongoConnection::tryDeleteManyByFilter(const std::string &DBName, const std::string &collection_name, bsoncxx::document::view filter)
{
  mongocxx::collection collection;
  if (!tryGetCollection(DBName, collection_name, collection)) return -1;
  bsoncxx::stdx::optional<mongocxx::result::delete_result> result;
  try {
    result = collection.delete_many({filter});
  } catch (mongocxx::exception& e) {
    std::cout << e.what() << std::endl;
    return 0;
  }
  if(result) return result->deleted_count();
  return 0;
}

bool BaseMongoConnection::tryDeleteOneByObjectID(const std::string &DBName, const std::string &collection_name,
                                                 bsoncxx::oid oid){
  using bsoncxx::builder::basic::kvp;
  auto filter = bsoncxx::builder::basic::document{};
  filter.append(kvp("_id", oid));
  return tryDeleteOneByFilter(DBName, collection_name, filter);
}

int BaseMongoConnection::tryDeleteManyByObjectIDs(const std::string &DBName, const std::string &collection_name,
                                                  const std::vector<bsoncxx::oid> &oids){
  using bsoncxx::builder::basic::kvp;
  auto filter = bsoncxx::builder::basic::document{};
  filter.append(kvp("_id", bsoncxx::types::b_document{ makeBsonObjectIDArrayQuery(oids).view() }));
  return tryDeleteManyByFilter(DBName, collection_name, filter);
}

bool BaseMongoConnection::IsConnected() const
{
  return _connected;
}

bsoncxx::document::value BaseMongoConnection::makeBsonObjectIDArrayQuery(const std::vector<bsoncxx::oid>& oids, const std::string& query_type) {
  using bsoncxx::builder::basic::kvp;
  using bsoncxx::builder::basic::sub_array;
  auto filter = bsoncxx::builder::basic::document{};
  filter.append(kvp(query_type, [&oids](sub_array bson_oids) {
      for (const auto& oid : oids) {
        bson_oids.append(oid);
      }
  }));
  return  filter.extract();
}

void BaseMongoConnection::CursorToMongoDocument(mongocxx::cursor &&cursor,
                                                const std::string& collection_name, const std::string& db_name,
                                                std::shared_ptr<std::vector<std::shared_ptr<MongoDocument>>>& mongo_docs, std::shared_ptr<MongoDocument> template_doc)
{
  for(auto doc : cursor){
    std::shared_ptr<MongoDocument> td = template_doc->MakeNewMongoDocument(doc, collection_name, db_name);
    mongo_docs->emplace_back(td);
  }
}

void BaseMongoConnection::CursorToMongoDocument(mongocxx::cursor &cursor,
                                                const std::string& collection_name, const std::string& db_name,
                                                std::shared_ptr<std::vector<std::shared_ptr<MongoDocument>>>& mongo_docs, std::shared_ptr<MongoDocument> template_doc)
{
  for(auto doc : cursor){
    std::shared_ptr<MongoDocument> td = template_doc->MakeNewMongoDocument(doc, collection_name, db_name);
    mongo_docs->emplace_back(td);
  }
}

void BaseMongoConnection::CursorToMongoDocument(mongocxx::cursor &&cursor,
                                                const std::string& collection_name, const std::string& db_name,
                                                std::shared_ptr<std::vector<std::shared_ptr<MongoGenericDocument>>>& mongo_docs, std::shared_ptr<MongoGenericDocument> template_doc)
{
  for(auto doc : cursor){
    std::shared_ptr<MongoGenericDocument> td = template_doc->MakeNewGenericDocument(doc, collection_name, db_name);
    mongo_docs->emplace_back(td);
  }
}

void BaseMongoConnection::CursorToMongoDocument(mongocxx::cursor &cursor,
                                                const std::string& collection_name, const std::string& db_name,
                                                std::shared_ptr<std::vector<std::shared_ptr<MongoGenericDocument>>>& mongo_docs, std::shared_ptr<MongoGenericDocument> template_doc)
{
  for(auto doc : cursor){
    std::shared_ptr<MongoGenericDocument> td = template_doc->MakeNewGenericDocument(doc, collection_name, db_name);
    mongo_docs->emplace_back(td);
  }
}

void BaseMongoConnection::PrintQueryOutput(mongocxx::cursor& cursor)
{
  for(auto doc : cursor) std::cout << bsoncxx::to_json(doc) << "\n";
  cursor.begin();
}

void BaseMongoConnection::PrintQueryOutput(mongocxx::cursor&& cursor)
{
  for(auto doc : cursor) std::cout << bsoncxx::to_json(doc) << "\n";
}

void BaseMongoConnection::PrintQueryOutput(bsoncxx::stdx::optional<bsoncxx::document::value> doc)
{
  if (doc)  std::cout << bsoncxx::to_json(*doc);
  std::cout << "\n";
}

