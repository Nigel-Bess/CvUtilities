//
// Created by amber on 7/27/20.
//

#include "FulfilMongoCpp/mongo_objects/mongo_array_element.h"
using ff_mongo_cpp::mongo_objects::MongoArrayElement;

#include "FulfilMongoCpp/mongo_objects/mongo_document_element.h"
using ff_mongo_cpp::mongo_objects::MongoDocumentElement;

#include "FulfilMongoCpp/mongo_objects/mongo_object_id.h"
using ff_mongo_cpp::mongo_objects::MongoObjectID;
using ff_mongo_cpp::mongo_objects::MongoElement;

MongoDocumentElement::MongoDocumentElement(bsoncxx::document::element elem_val){
  this->raw_element = elem_val;
}

int MongoDocumentElement::tryGetValue(std::vector<std::string>& container){ return tryGetValueHelper(container); }
int MongoDocumentElement::tryGetValue(std::vector<int>& container){ return tryGetValueHelper(container); }
int MongoDocumentElement::tryGetValue(std::vector<double>& container){ return tryGetValueHelper(container); }
int MongoDocumentElement::tryGetValue(std::vector<bool>& container){ return tryGetValueHelper(container); }
int MongoDocumentElement::tryGetValue(std::vector<MongoObjectID>& container){ return tryGetValueHelper(container); }

int MongoDocumentElement::tryGetValue(std::string& container){ return MongoElement::tryGetElem(this->raw_element, container);  }
int MongoDocumentElement::tryGetValue(int& container){ return MongoElement::tryGetElem(this->raw_element, container);  }
int MongoDocumentElement::tryGetValue(double& container){ return MongoElement::tryGetElem(this->raw_element, container);  }
int MongoDocumentElement::tryGetValue(bool& container){ return MongoElement::tryGetElem(this->raw_element, container);  }
int MongoDocumentElement::tryGetValue(MongoObjectID& container){ return MongoElement::tryGetElem(this->raw_element, container); }

std::shared_ptr<MongoElement> MongoDocumentElement::getNext(const std::string& index)
{
  return std::make_shared<MongoDocumentElement>(this->raw_element[index]);
}

std::shared_ptr<MongoElement> MongoDocumentElement::getNext(int index)
{
  return std::make_shared<MongoArrayElement>(this->raw_element[index]);
}

std::string MongoDocumentElement::type(){
  return "MongoDocumentElement";
}