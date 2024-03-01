//
// Created by amber on 7/26/20.
//

#include <ctime>
#include "FulfilMongoCpp/mongo_objects/mongo_object_id.h"
using ff_mongo_cpp::mongo_objects::MongoObjectID;


MongoObjectID::MongoObjectID(const std::string& oid){
  try {
    this->oid = bsoncxx::oid(oid);
  } catch (bsoncxx::exception& be) {
    this->oid = bsoncxx::oid("000000000000000000000000");
  }
}

MongoObjectID::MongoObjectID(const bsoncxx::oid& oid){
  this->oid = oid;
}

MongoObjectID::MongoObjectID(int year, int month, int day,
        int hour, int min, int sec, bool use_local, const std::string& identifier)
{
  try {
    this->oid = bsoncxx::oid(get_hextime(year, month, day, hour, min, sec, use_local) + identifier);
  } catch (bsoncxx::exception& be) {
    this->oid = bsoncxx::oid("000000000000000000000000");
  }
}

std::vector<bsoncxx::oid> MongoObjectID::convertToBSonIdVec(const std::vector<std::string>& oids){
  return convertIdVec<bsoncxx::oid>(oids);
}

std::vector<bsoncxx::oid> MongoObjectID::convertToBSonIdVec(const std::vector<MongoObjectID>& oids){
  return convertIdVec<bsoncxx::oid>(oids);
}

std::vector<MongoObjectID> MongoObjectID::convertToMongoIdVec(const std::vector<std::string>& oids){
  return convertIdVec<MongoObjectID>(oids);
}

std::vector<MongoObjectID> MongoObjectID::convertToMongoIdVec(const std::vector<bsoncxx::oid>& oids){
  return convertIdVec<MongoObjectID>(oids);
}

void MongoObjectID::updateID() {
  this->oid = bsoncxx::oid();
}

std::time_t MongoObjectID::get_time_t() const{
  return this->oid.get_time_t();
}

std::string MongoObjectID::get_time(bool use_local) const{
  std::time_t t = this->get_time_t();
  std::stringstream time;
  if (use_local){
     time << std::put_time(std::localtime(&t), "%F %T %z");
    return time.str();
  }
  time << std::put_time(std::gmtime(&t), "%F %T %z");
  return time.str();
}

std::string MongoObjectID::get_hextime(int year, int month, int day, int hour, int min, int sec, bool use_local)
{
  std::time_t rawtime;
  std::time ( &rawtime );
  struct std::tm * timeinfo;
  timeinfo = (use_local)? localtime(&rawtime): gmtime(&rawtime);
  timeinfo->tm_year = year - 1900;
  timeinfo->tm_mon = month - 1;
  timeinfo->tm_mday = day;
  timeinfo->tm_hour = hour;   timeinfo->tm_min = min; timeinfo->tm_sec = sec;

  std::time_t supplied_time = mktime (timeinfo);
  std::stringstream timestream;
  timestream << std::hex << supplied_time;
  return timestream.str();

}





bool MongoObjectID::is_null() const
{
  return (this->oid.to_string() == "000000000000000000000000");
}
