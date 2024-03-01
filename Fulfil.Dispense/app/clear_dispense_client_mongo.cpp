//
// Created by amber on 12/17/20.
//

#include "FulfilMongoCpp/mongo_filter/mongo_filters.h"
#include "FulfilMongoCpp/mongo_connection.h"
#include "FulfilMongoCpp/mongo_objects/mongo_object_id.h"

using ff_mongo_cpp::mongo_objects::MongoObjectID;
using ff_mongo_cpp::mongo_filter::MongoFilter;
using ff_mongo_cpp::MongoConnection;

int main(int argc, char** argv)
{

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
  std::cout << "Deleting all entries in Logging.TrayCounts from dispense_test_client.cpp\n";
  std::cout << "This is any entry with a primaryKeyID that equals:\n\t111111111111111111111111\n\t555555555555555555555555\n\t666666666666666666666666\n";

  std::vector<MongoObjectID> dispense_test_query_ids = {
          MongoObjectID("111111111111111111111111"),
          MongoObjectID("555555555555555555555555"),
          MongoObjectID("666666666666666666666666")
  };

  MongoFilter dispense_test_query_filter = MongoFilter("primaryKeyID", MongoFilter::FilterType::In, dispense_test_query_ids);
  std::cout << "Successfully deleted " <<  conn.tryDeleteManyByFilter("Logging", "TrayCounts", dispense_test_query_filter)
            << " entries from Logging.TrayCounts!" << std::endl;
  return 0;

}

