#include "FulfilMongoCpp/mongo_filter/mongo_filters.h"
#include "FulfilMongoCpp/mongo_objects/mongo_document.h"
#include "FulfilMongoCpp/base_mongo_connection.h"
#include "FulfilMongoCpp/mongo_connection.h"
#include "FulfilMongoCpp/mongo_objects/mongo_object_id.h"
#include "FulfilMongoCpp/mongo_objects/mongo_basic_document.h"
#include "FulfilMongoCpp/mongo_objects/mongo_scoped_document.h"
#include "FulfilMongoCpp/mongo_objects/mongo_document_element.h"
#include "FulfilMongoCpp/mongo_objects/mongo_array_element.h"
#include "FulfilMongoCpp/mongo_objects/mongo_element.h"
#include "FulfilMongoCpp/mongo_parse/mongo_bsonxx_parser.h"
#include "FulfilMongoCpp/mongo_parse/mongo_bsonxx_encoder.h"
#include <unistd.h>
#include <ctime>

#if (DISABLE_JSON==0)
#include "FulfilMongoCpp/mongo_json/mongo_json_document.h"
using ff_mongo_cpp::mongo_objects::MongoJsonDocument;
#endif

using ff_mongo_cpp::mongo_parse::MongoBsonxxEncoder;
using ff_mongo_cpp::mongo_parse::MongoBsonxxParser;
using ff_mongo_cpp::mongo_objects::MongoElement;
using ff_mongo_cpp::mongo_objects::MongoArrayElement;
using ff_mongo_cpp::mongo_objects::MongoDocumentElement;
using ff_mongo_cpp::mongo_objects::MongoDocument;
using ff_mongo_cpp::mongo_objects::MongoGenericDocument;
using ff_mongo_cpp::mongo_objects::MongoBasicDocument;
using ff_mongo_cpp::mongo_objects::MongoScopedDocument;
using ff_mongo_cpp::mongo_objects::MongoObjectID;
using ff_mongo_cpp::mongo_filter::MongoFilter;
using ff_mongo_cpp::MongoConnection;
using ff_mongo_cpp::BaseMongoConnection;

int main(int argc, char** argv)
{

  /*
  std::string conn_str;

  if(argc < 2)
  {
    // If no arg provided uses production config as default
    conn_str = "mongodb://localhost:27018";
  } else {
    conn_str = argv[1]; //
  }
  std::cout << "Attempting to connect to " << conn_str << "\n";

  MongoConnection conn = MongoConnection(conn_str);
  std::cout << "Able to connect to mongo: " << conn.IsConnected() << "\n";
  std::cout << "Testing Oid, previously null id ";
  MongoObjectID test_id = MongoObjectID();
  std::cout << test_id.str() << " is now a new ID ";
  test_id.updateID();
  std::cout << test_id.str() << " created at time " << test_id.get_time() << std::endl;
  std::time_t start_time = test_id.get_time_t();


  MongoFilter mf = MongoFilter("LaneCount", MongoFilter::FilterType::Eq, 1);

  std::vector<std::string> fs = {"928222060178", "839212060149" };
  MongoFilter mf_comp = MongoFilter("camera_serial_number", MongoFilter::FilterType::In, fs);
  mf_comp.appendToFilter(MongoFilter::CompositeType::Or, "camera_serial_number",
          MongoFilter::FilterType::Eq,"839112061089");

    MongoFilter mf_comp2 = MongoFilter("camera_type", MongoFilter::FilterType::Eq, "tester");
    mf_comp2.appendToFilter(MongoFilter::CompositeType::Or, "camera_type",
                           MongoFilter::FilterType::Eq,"Drop");
    std::vector<MongoFilter> comp_filters = {mf_comp, mf_comp2};

    MongoFilter multi_comp = MongoFilter(MongoFilter::CompositeType::And, comp_filters);
  std::cout << "Composite filter: \n" << multi_comp.str() << std::endl << std::endl;


  std::shared_ptr<std::vector<std::shared_ptr<MongoGenericDocument>>> many_found = std::make_shared<std::vector<std::shared_ptr<MongoGenericDocument>>>();
#if (DISABLE_JSON==0)
  std::shared_ptr<MongoGenericDocument> generic_template = std::make_shared<MongoJsonDocument>();
#else
  std::shared_ptr<MongoGenericDocument> generic_template = std::make_shared<MongoBasicDocument>();
#endif
    bool found_many = conn.tryFindByFilter("Machines", "DepthCameras", multi_comp, many_found, generic_template);
    std::cout << "Able to find items matching Composite filter: " << found_many <<  "\n";

    std::vector<MongoObjectID> oids;
    for (auto d : *many_found){
      std::cout << "Found document "<< d->GetObjectID().str() << std::endl;
      std::cout << d->str() << std::endl;
      auto res2 = conn.findOneByObjectID("Machines", "DepthCameras", d->GetObjectID().bson_oid());
      std::cout << "should be same as above with oid " << std::endl;
      BaseMongoConnection::PrintQueryOutput(res2);
      oids.emplace_back(d->GetObjectID());
    }
    std::cout << "\nDone with Composite filter!\n\n";
    std::shared_ptr<std::vector<std::shared_ptr<MongoDocument>>> many_found_id = std::make_shared<std::vector<std::shared_ptr<MongoDocument>>>();
#if (DISABLE_JSON==0)
    std::shared_ptr<MongoJsonDocument> found_many_id_template = std::make_shared<MongoJsonDocument>();
#else
  std::shared_ptr<MongoBasicDocument> found_many_id_template = std::make_shared<MongoBasicDocument>();
#endif

    bool found_with_id_vec = conn.tryFindByObjectIDs("Machines", "DepthCameras", oids, many_found_id, found_many_id_template);
    std::cout << "Able to find same items with id vec: " << found_with_id_vec <<  "\n";
    std::cout << "Should b same items as Composite filter\n";
    for (auto d : *many_found_id){
    std::cout << "Found document "<< d->GetObjectID().str() << std::endl;
#if (DISABLE_JSON==0)
      MongoJsonDocument* typed = dynamic_cast<MongoJsonDocument *>(&(*d));
#else
      MongoBasicDocument* typed = dynamic_cast<MongoBasicDocument *>(&(*d));
#endif
      std::cout << typed->str() << std::endl;
    }



    std::cout << "\nTrying value insert: \n";
    std::shared_ptr<std::vector<std::shared_ptr<MongoDocument>>> basic_docs = std::make_shared<std::vector<std::shared_ptr<MongoDocument>>>();
    MongoFilter basic_mf = MongoFilter("camera_serial_number", MongoFilter::FilterType::Eq, "928222063615");
    mongocxx::cursor basic_mfc = conn.findManyByFilter("Machines", "DepthCameras", basic_mf.view());


    basic_mfc = conn.findManyByFilter("Machines", "DepthCameras", basic_mf.view());
    std::shared_ptr<MongoBasicDocument> basic_template = std::make_shared<MongoBasicDocument>();
    conn.CursorToMongoDocument(basic_mfc,"Machines", "DepthCameras", basic_docs, basic_template);
    std::cout << "\nNumber of basic Docs to insert is:"<< basic_docs->size()<<" \n";


    std::cout << "\nExample basic Doc to insert:"<< (dynamic_cast<MongoBasicDocument *>(&(*basic_docs->at(0))))->str() <<" \n";
    std::vector<MongoObjectID> oid_b;
    std::cout << "\nNumber of basic Docs successfully inserted: "<< conn.tryInsertMany("Machines", "DepthCameras", basic_docs, oid_b) << ", listed below:\n";
    for (auto d : oid_b)
    std::cout << "\tInserted Doc:"<< d.str() << std::endl;
    std::cout << "\n";



    mf = MongoFilter("camera_serial_number", MongoFilter::FilterType::Eq, "928222063615");
    mongocxx::cursor mfc = conn.findManyByFilter("Machines", "DepthCameras", mf.view());

    std::cout << "\nConvert to doc: \n";
    std::shared_ptr<std::vector<std::shared_ptr<MongoGenericDocument>>> serial_docs = std::make_shared<std::vector<std::shared_ptr<MongoGenericDocument>>>();
#if (DISABLE_JSON==0)
  std::shared_ptr<MongoGenericDocument> template_doc = std::make_shared<MongoJsonDocument>();
#else
  std::shared_ptr<MongoGenericDocument> template_doc = std::make_shared<MongoBasicDocument>();
#endif
     //changed from generic
    conn.CursorToMongoDocument(mfc,"Machines", "DepthCameras", serial_docs, template_doc);

    std::cout << "\nFound all entries with serial 928222063615! Num entries: " << serial_docs->size() <<"\n";
    for (auto d : *serial_docs){
      std::cout << "Found document "<< d->GetObjectID().str() <<" created at "<< d->GetObjectID().get_time() << std::endl;
      std::cout << d->str() << std::endl;
    }

    MongoObjectID oid;
  MongoObjectID oid1;
  MongoObjectID oid2;
  conn.tryInsertOne("Machines", "DepthCameras", (*basic_docs)[0], oid1);
  std::cout << "Oid of insert 1: " << std::string(oid1) << std::endl;
#if (DISABLE_JSON==0)
    MongoJsonDocument* can_edit = dynamic_cast<MongoJsonDocument *>(&(*serial_docs->at(0)));

    can_edit->data["camera_serial_number"] = "111111111111";
    can_edit->data["VLS"] = {};

    std::cout << "camera_type: " << can_edit->data["camera_type"].get<std::string>() << std::endl;
    can_edit->data["camera_type"] = "tester";
    can_edit->data["additional data"] = "More data";

    std::cout << "Edited document "<< can_edit->GetObjectID().str() << std::endl;
    std::cout << can_edit->str() << std::endl;
    std::cout << "COMPLETE!" << std::endl;

    conn.tryInsertOne("Machines", "DepthCameras", std::make_shared<MongoJsonDocument>(*can_edit), oid2);
    std::cout << "Oid of insert 2: " << std::string(oid2) << std::endl;
#else
  conn.tryInsertOne("Machines", "DepthCameras", (*basic_docs)[0], oid2);
  std::cout << "Oid of insert 2: " << std::string(oid2) << std::endl;
#endif




  bsoncxx::document::value test_doc_value = bsoncxx::builder::stream::document{}
          << "Test" << "Document"
          << "Addition" << bsoncxx::builder::stream::open_array
          << 1 << 2 << 3
          << bsoncxx::builder::stream::close_array
          << "subdoc" << bsoncxx::builder::stream::open_document
          << "x" << 203
          << "y" << 102
          << bsoncxx::builder::stream::close_document
          << bsoncxx::builder::stream::finalize;

  MongoBsonxxEncoder mbe = MongoBsonxxEncoder();
  mbe.addField("Check", "Updates");
  mbe.addField("Random", "Additions");
  mbe.addField("Int", 2);
  mbe.addField("double", 2.2);
  mbe.addField("Mongo_id", test_id);
  std::vector<std::string> things = {"testing", "vector", "interface"};
  mbe.addField("add_vector", things);
  mbe.addField("Full_document", test_doc_value.view());
  std::vector<std::vector<int>> vv = { {1, 2}, {3, 4} };
  //std::vector<int> vv = { 1, 2, 3, 4 };
  mbe.addField("vector_of_vec", vv);
  MongoObjectID test_id2 = MongoObjectID();
  test_id2.updateID();
  std::vector<std::vector<MongoObjectID>> idid = { {test_id, MongoObjectID()}, {test_id2, test_id} };
  //std::vector<MongoObjectID> idid = { test_id, MongoObjectID()};
  mbe.addField("vector_of_vec_of_id", idid);

  bsoncxx::document::value concat_doc = bsoncxx::builder::stream::document{}
          << "Test" << "Concat"
          << "Of" << "Document"
          << bsoncxx::builder::stream::finalize;
  mbe.concatenate(concat_doc);
  std::vector<std::string> keys = {"key1", "key2", "key3", "key4"};
  std::vector<float> vals = {1.1, 2.2, 3.3, 4.4};
  mbe.bulkAddField(keys, vals);
  MongoBsonxxEncoder sub = MongoBsonxxEncoder();
  sub.bulkAddField({"Sub1", "Sub2"}, std::vector<int>({1, 2}));
  mbe.addField( "Rval", sub.extract().view());

  bsoncxx::document::value final_doc = mbe.extract();
  std::cout << bsoncxx::to_json(final_doc.view()) << std::endl;


  auto builder = bsoncxx::builder::stream::document{};
  bsoncxx::document::value doc_value = builder
          << "Test" << "Round 2"
          << "Check" << "Updates"
          << "Number" << 0
          << "Array" << bsoncxx::builder::stream::open_array
          << 1 << 2 << 3
          << bsoncxx::builder::stream::close_array
          << "info" << bsoncxx::builder::stream::open_document
          << "x" << 203
          << "y" << 102
          << bsoncxx::builder::stream::close_document
          << bsoncxx::builder::stream::finalize;

  std::shared_ptr<MongoGenericDocument> md_update = std::make_shared<MongoBasicDocument>(final_doc, "DepthCameras", "Machines");

MongoElement* e1;
MongoElement* e2;
MongoDocumentElement ed1 = MongoDocumentElement(doc_value.view()["Number"]);
MongoArrayElement ed2 = MongoArrayElement(doc_value.view()["Array"][0]);

e1 = &ed1;
e2 = &ed2;
int out;
e1->tryGetValue(out);
std::cout << "Test 1 should be 0 : Output " << out << std::endl;
e2->tryGetValue(out);
std::cout << "Test 2 should be 1 : Output " << out << std::endl;

std::cout << "Test 3 should be 0 : Output " << ed1.get<int>() << std::endl;
std::cout << "Test 4 should be 1 : Output " << ed2.get<int>() << std::endl;

MongoBsonxxParser mbp = MongoBsonxxParser(doc_value.view());

std::cout << "Test 5 should be Updates : Output " << mbp.getFromRoot<std::string>("Check") << std::endl;

std::vector<int> outv;
mbp.parseFromRoot(out, "Array", 2);
std::cout << "Test 6 should be 3 : Output " << out << std::endl;
mbp.parseFromRoot(outv, "Array");
std::cout << "Test 7 should be 3 : Output " << outv.size() << ". Output should be 2: "<< outv[1] <<std::endl;
std::cout << "Test 8 should be 102 : Output " << mbp.getFromRoot<int>("info", "y") << ". " << std::endl;
MongoDocumentElement ed3 = MongoDocumentElement(doc_value.view()["Array"]);
std::shared_ptr<MongoElement> arr = ed3.getNext(2);
arr->tryGetValue(out);
std::cout << "Test 9 should be 3 : Output " << out << "."<<std::endl;
std::cout << "Test 10 should be 3 : Output " << mbp.getFromRoot<int>("Array", 2) << ". " << std::endl;

std::cout << "Test 11 should be 1 : Output " << mbp.getFromRoot<std::vector<int>>("Array")[0] << ". " << std::endl;
mbp.moveCursorToElem("Array");
std::cout << "Test 12 should be 1 : Output " << mbp.get<std::vector<int>>()[0] << ". " << std::endl;
mbp.moveCursorToElem(0);
std::cout << "Test 13 should be 1 : Output " << mbp.get<int>() << ". " << std::endl;
mbp.parseCurrent(out);
std::cout << "Test 14 should be 1 : Output " << out << ". " << std::endl;

MongoBasicDocument basic = MongoBasicDocument(doc_value, "test", "test");
std::cout << "Made document" << std::endl;
basic.parseFromRoot(out, "Array", 2);
std::cout << "Test 15 should be 3 : Output " << out << std::endl;
std::cout << "Test 16 should be 1 : Output " << basic.getFromRoot<std::vector<int>>("Array")[0] << ". " << std::endl;
std::cout << "Test 17 should be 3 : Output " <<  basic.getFromRoot<int>("Array", 2) << ". " << std::endl;

basic.parseFromRoot(out, "Array", 2);
std::cout << "Test 18 should be 3 : Output " << out << std::endl;
std::cout << "Test 19 should be 1 : Output " << basic.getFromRoot<std::vector<int>>("Array")[0] << ". " << std::endl;
std::cout << "Test 20 should be 3 : Output " <<  basic.getFromRoot<int>("Array", 2) << ". " << std::endl;

std::vector<std::string> os;
os = mbp.bulkGet<std::string>({"Test", "Check"});
std::cout << "Test 21 should be Round2...Updates : Output " <<  os[0] << "..." << os[1] << ". " << std::endl;

serial_docs->clear();
oids.clear();
oids = {oid1, oid2};
MongoFilter mf2 = MongoFilter("_id", MongoFilter::FilterType::In, oids);

conn.tryFindByFilter("Machines", "DepthCameras", mf2, serial_docs, template_doc);
std::cout << "Find Length of documents list: " << serial_docs->size() << std::endl;

std::cout << "\n\nUpdate by filter testing\n "<< std::endl;
std::cout << bsoncxx::to_json(md_update->MakeWritableValue().view()) << std::endl;
std::cout << "Number of items in updates: " << conn.tryUpdateManyByFilter("Machines", "DepthCameras",
                                                                        mf2, md_update, "$set") << std::endl;

std::cout << "\n\nUpdate by OIDs testing\n "<< std::endl;
std::cout << bsoncxx::to_json(doc_value.view()) << std::endl;
std::shared_ptr<MongoGenericDocument> md_update2 = std::make_shared<MongoBasicDocument>(doc_value, "DepthCameras", "Machines");
std::cout << "Number of items in updates: "
<< conn.tryUpdateManyByObjectIDs("Machines", "DepthCameras", md_update2, oids) << std::endl;


  int year=2020;
  int month=12;
  int day=1;
  int hour=14;
  int min=24;
  int sec=04;
  MongoObjectID start_id = MongoObjectID(year, month, day, hour, min, sec);
  MongoObjectID end_id = MongoObjectID(year, month, 30, hour, 28, 03);
  MongoFilter time_filter = MongoFilter("_id", MongoFilter::FilterType::Gte, start_id);
  time_filter.appendToFilter(MongoFilter::CompositeType::And, "_id", MongoFilter::FilterType::Lte, end_id);
  std::cout << "Temporal Object ID Filter"<< std::endl;
  std::cout << time_filter.str() << std::endl;



  std::shared_ptr<std::vector<std::shared_ptr<MongoGenericDocument>>> time_slice = std::make_shared<std::vector<std::shared_ptr<MongoGenericDocument>>>();
  std::shared_ptr<MongoGenericDocument> template100 = std::make_shared<MongoBasicDocument>();
  conn.tryFindByFilter("Machines", "DepthCameras", time_filter, time_slice, template100);

  std::cout << "\nNum Docs found at time slice: " << time_slice->size()<< ", using filter "<< time_filter.str() <<"\n";
  for (auto d : *time_slice){
    std::cout << "Found document "<< d->GetObjectID().str() <<" created at "<< d->GetObjectID().get_time() << std::endl;
    std::cout << d->str() << std::endl;
  }
  std::cout << "\nEnd of time slice: " << std::endl;

  MongoFilter mf3 = MongoFilter("Check", MongoFilter::FilterType::Eq, "Updates");
  MongoFilter mf4 = MongoFilter("_id", MongoFilter::FilterType::Gt, test_id);
std::cout << "6 second wait before delete." << std::endl;
int usec_to_sec = 1000000;
usleep(6*usec_to_sec);
std::cout << "Successfully deleted: " <<  conn.tryDeleteManyByFilter("Machines", "DepthCameras", mf3)
          << " using filter " << mf3.str() << std::endl;
  std::cout << "Successfully deleted: " <<  conn.tryDeleteManyByFilter("Machines", "DepthCameras", mf4)
            << " using filter "<< mf4.str() << std::endl;
*/
  return 0;

}
