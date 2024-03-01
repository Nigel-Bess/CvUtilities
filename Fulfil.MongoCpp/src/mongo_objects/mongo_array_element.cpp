//
// Created by amber on 7/27/20.
//

#include <memory>

#include "FulfilMongoCpp/mongo_objects/mongo_document_element.h"
using ff_mongo_cpp::mongo_objects::MongoDocumentElement;

#include "FulfilMongoCpp/mongo_objects/mongo_element.h"
using ff_mongo_cpp::mongo_objects::MongoElement;

#include "FulfilMongoCpp/mongo_objects/mongo_object_id.h"
using ff_mongo_cpp::mongo_objects::MongoObjectID;

#include "FulfilMongoCpp/mongo_objects/mongo_array_element.h"
using ff_mongo_cpp::mongo_objects::MongoArrayElement;

MongoArrayElement::MongoArrayElement(bsoncxx::array::element elem_val){
  this->raw_element = elem_val;
}

int MongoArrayElement::tryGetValue(std::vector<std::string>& container){ return tryGetValueHelper(container); }
int MongoArrayElement::tryGetValue(std::vector<int>& container){ return tryGetValueHelper(container); }
int MongoArrayElement::tryGetValue(std::vector<double>& container){ return tryGetValueHelper(container); }
int MongoArrayElement::tryGetValue(std::vector<bool>& container){ return tryGetValueHelper(container); }
int MongoArrayElement::tryGetValue(std::vector<MongoObjectID>& container){ return tryGetValueHelper(container); }

int MongoArrayElement::tryGetValue(std::string& container){ return MongoElement::tryGetElem(this->raw_element, container);  }
int MongoArrayElement::tryGetValue(int& container){ return MongoElement::tryGetElem(this->raw_element, container);  }
int MongoArrayElement::tryGetValue(double& container){ return MongoElement::tryGetElem(this->raw_element, container);  }
int MongoArrayElement::tryGetValue(bool& container){ return MongoElement::tryGetElem(this->raw_element, container);  }
int MongoArrayElement::tryGetValue(MongoObjectID& container){ return MongoElement::tryGetElem(this->raw_element, container);  }

std::shared_ptr<MongoElement> MongoArrayElement::getNext(const std::string& index)
{
  return std::make_shared<MongoDocumentElement>(this->raw_element[index]);
}

std::shared_ptr<MongoElement> MongoArrayElement::getNext(int index)
{
  return std::make_shared<MongoArrayElement>(this->raw_element[index]);
}

std::string MongoArrayElement::type(){
  return "MongoArrayElement";
}