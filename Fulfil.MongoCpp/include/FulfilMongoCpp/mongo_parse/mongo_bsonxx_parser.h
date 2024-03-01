#ifndef FULFILMONGOCPP_MONGO_BSONCXX_PARSER_H_
#define FULFILMONGOCPP_MONGO_BSONCXX_PARSER_H_

#include <memory>

#include "FulfilMongoCpp/mongo_objects/mongo_array_element.h"
using ff_mongo_cpp::mongo_objects::MongoArrayElement;

#include "FulfilMongoCpp/mongo_objects/mongo_document_element.h"
using ff_mongo_cpp::mongo_objects::MongoDocumentElement;

namespace ff_mongo_cpp {
    namespace mongo_parse {
        class MongoBsonxxParser {
        public:
            MongoBsonxxParser(bsoncxx::document::view doc);
            MongoBsonxxParser();

            template<typename T>
            std::vector<T> bulkGet(std::vector<std::string> keys, std::shared_ptr<std::vector<int>> errors = nullptr)
            {
              int num_elems = keys.size();
              std::vector<T> containers;
              for (int i = 0; i < num_elems; i++){
                T container;
                if (!parseFromRoot(container, keys[i])) errors->emplace_back(i);
                containers.emplace_back(container);
              }
              return containers;
            }

            //Todo, should probably allow it to accept a map
            template<typename T>
            int bulkParse(std::shared_ptr<std::vector<std::string>>  keys,
                          std::vector<T*> containers, //containers should be ref pointers
                          std::vector<int>& errors)
            {
                int num_elems = keys->size();
                if (num_elems != containers.size())
                    throw std::runtime_error("Mismatch in number of keys and containers provided in bulkParse!");
                for (int i = 0; i < num_elems; i++){
                    if (!parseFromRoot(*(containers.at(i)), keys->at(i))) errors.emplace_back(i);
                }
                return errors.size();
            }

            template<typename T>
            int bulkParse(std::shared_ptr<std::vector<std::string>>  keys,
                          std::vector<T*> containers) //containers should be ref pointers
            {
              int num_elems = keys->size();
              int errors = 0;
              if (num_elems != containers.size())
                throw std::runtime_error("Mismatch in number of keys and containers provided in bulkParse!");
              for (int i = 0; i < num_elems; i++){
                if (!parseFromRoot(*(containers.at(i)), keys->at(i))) errors++;
              }
              return errors;
            }


            template<typename T, typename... Args>
            int parseFromRoot(T&& container, const std::string& key, Args&&... args)
            {
              this->reset();
              moveCursorToElem(key, std::forward<Args>(args)...);
              bool success = this->current_element->tryGetValue(std::forward<T>(container));
              this->reset();
              return success;
            }

            template<typename T>
            int parseCurrent(T&& container)
            {
              if (cursorAtRoot())
                throw std::runtime_error("Cursor is currently at root. Please supply an key!");
              bool success = this->current_element->tryGetValue(std::forward<T>(container));
              return success;
            }

            template<typename... Args>
            void moveCursorToElem(const std::string& key, Args&&... args) {
              if (cursorAtRoot()) {
                this->current_element = std::make_shared<ff_mongo_cpp::mongo_objects::MongoDocumentElement>(this->document[key]);
              } else {
                std::shared_ptr<MongoElement> next = this->current_element->getNext(key);
                this->current_element = nullptr;
                this->current_element = std::move(next);
              }
              moveCursorToElem(std::forward<Args>(args)...);
            }

            template<typename... Args>
            void moveCursorToElem(int key, Args&&... args) {
              if (cursorAtRoot()) {
                throw std::runtime_error("Cursor is at root, first index value must be a string index, not array index!");
              }
              std::shared_ptr<MongoElement> next = this->current_element->getNext(key);
              this->current_element = nullptr;
              this->current_element = std::move(next);
              moveCursorToElem(std::forward<Args>(args)...);
            }

            template<typename T, typename F, typename... Args>
            T getFromRoot(F&& first, Args&&... args) {
              T container;
              if (!this->parseFromRoot(container, std::forward<F>(first), std::forward<Args>(args)...))
              {
                std::stringstream err_stream;
                err_stream << "Issue getting parse value of key " << first
                            << "! Check container type and if key is present in document.";
                throw std::runtime_error(err_stream.str());
              }

              return container;
            }

            template<typename T>
            T get() {
              T container;
              if (cursorAtRoot())
                throw std::runtime_error("Cursor is currently at root. Please supply an key!");
              if (!this->current_element->tryGetValue(container))
                throw std::runtime_error("Issue getting value at current cursor position. Check that your typing is correct.");
              return container;
            }

            void reset();
            bool cursorAtRoot();

        protected:
            void moveCursorToElem() {
              if (cursorAtRoot())
                throw std::runtime_error(
                        "Cursor is at root, a initial string index must be provided!");
            }

        private:
            bsoncxx::document::view document;
            std::shared_ptr<MongoElement> current_element;


        };
    }
}


#endif //FULFILMONGOCPP_MONGO_BSONCXX_PARSER_H_

