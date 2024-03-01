#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <utility>
#include <vector>
#include <cstring>
#include <dirent.h>
#include "Fulfil.CPPUtils/file_system_util.h"

using fulfil::utils::FileSystemUtil;


bool FileSystemUtil::directory_exists(const char *directory) {
    struct stat s;
    return stat(directory, &s) == 0 && S_ISDIR(s.st_mode);
}

int FileSystemUtil::directory_empty(const std::string& directory)
{
  return directory_empty(directory.c_str());
}

int FileSystemUtil::directory_empty(const std::shared_ptr<std::string>&directory){
  return directory_empty(directory->c_str());
}

int FileSystemUtil::directory_empty(const char *directory)
{
  using std::strcmp;
  int found_fs_obj = 0;

  DIR *dir = opendir(directory);
  dirent *dp;
  if (dir == NULL) return errno;
  // If dir exists and is empty expect found_fs_obj == 2 for . and ..
  while (found_fs_obj < 3 && (dp = readdir(dir)) != nullptr) {
    found_fs_obj++;
  }
  closedir(dir);
  if (found_fs_obj == 2) return 0;
  return -1;
}

void FileSystemUtil::create_directory(const char *directory) {
    std::string command;
    command.append("mkdir ");
    command.append(directory);
    system(command.c_str());
}

void FileSystemUtil::create_nested_directory(const std::shared_ptr<std::string>&directory) {
  // TODO either update to use std::filesystem or ensure that there is an
  //  implicit R-value move that prevents unnecessary copies
  create_nested_directory(*directory);
}

void FileSystemUtil::create_nested_directory(const std::string& directory) {
  // Validate that user dir is the same
  if (directory.find("/home") == 0){
    if (directory.length() < 6) throw std::runtime_error("You tried to create home directory. Illegal!");
    if (directory.at(5) == '/'){
      std::size_t end_usr_dir = directory.find('/', 6);
      if (!directory_exists((directory).substr(0, end_usr_dir).c_str()))
        throw std::runtime_error("You tried to create a directory along a new user path in home. Illegal!");
    }
  }
  std::string command;
  command.append("mkdir -p ");
  command.append((directory));
  system(command.c_str());
}

bool FileSystemUtil::file_exists(const char *filename) {
    std::ifstream file(filename);
    bool is_good = file.good();
    file.close();
    return is_good;
}

void FileSystemUtil::write_to_file(const char *data, long data_size, const char *filename)
{
  std::ofstream file;
  file.open(filename);
  file.write(data, data_size);
  file.close();
}

std::shared_ptr<std::string> FileSystemUtil::get_string_from_file(const char *filename) {
    std::ifstream input_file(filename);
    std::shared_ptr<std::string> contents =std::make_shared<std::string>((std::istreambuf_iterator<char>(input_file)),
                                                      std::istreambuf_iterator<char>());
    input_file.close();
    return contents;
}

std::string fulfil::utils::FileSystemUtil::get_string_from_file(
    const std::string &filename) {
  std::ifstream input_file(filename);
  std::string contents = std::string((std::istreambuf_iterator<char>(input_file)),
                                                                        std::istreambuf_iterator<char>());
  input_file.close();
  return contents;
}

void FileSystemUtil::create_dir_if_not_exist(const char *dirname) {
    if (!directory_exists(dirname)) {
        create_directory(dirname);
    }
}

std::shared_ptr<std::string> FileSystemUtil::create_datetime_string(bool date_only)
{
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];
  time (&rawtime);
  timeinfo = localtime(&rawtime);
  const char* fmt = (date_only) ? "%Y_%m_%d" : "%Y_%m_%d_H%H_M%M_S%S";
  strftime(buffer,sizeof(buffer),fmt,timeinfo);
  std::shared_ptr<std::string> time_stamp_string = std::make_shared<std::string>(buffer);
  return time_stamp_string;
}

void FileSystemUtil::write_to_file_if_not_exists(const char * data, long data_size, const char * filename)
{
  if(!file_exists(filename))
  {
    write_to_file(data, data_size, filename);
  }
}

std::string FileSystemUtil::get_extension(const std::string& filename)
{
    std::size_t found = filename.find_last_of("./");
    if (found == std::string::npos || filename.at(found) == '/') return "";
    return filename.substr(found);
}

std::pair<std::string, std::string> FileSystemUtil::split_basedir(const std::shared_ptr<std::string>& path_name)
{
  if (path_name->back() == '/') return std::make_pair(path_name->substr(0,path_name->length() - 1), "");
  std::size_t found = path_name->find_last_of("/\\", path_name->length() - 2);
  if (found == std::string::npos) std::make_pair("", *path_name);
  return std::make_pair(path_name->substr(0,found), path_name->substr(found+1));
}

std::pair<std::string, std::string> FileSystemUtil::split_basedir(const std::string& path_name)
{
    if (path_name.back() == '/') return std::make_pair(path_name.substr(0,path_name.length() - 1), "");
    std::size_t found = path_name.find_last_of("/\\", path_name.length() - 2);
    if (found == std::string::npos) std::make_pair("", path_name);
    return std::make_pair(path_name.substr(0,found), path_name.substr(found+1));
}

void FileSystemUtil::make_containing_directory(const std::shared_ptr<std::string>& path_name)
{
  std::pair<std::string, std::string>  split_path = split_basedir(path_name);
  if (!split_path.first.empty() && !directory_exists(split_path.first.c_str())){
    create_nested_directory(std::make_shared<std::string>(split_path.first));
  }
}

std::shared_ptr<std::vector<std::shared_ptr<std::string>>> FileSystemUtil::get_files_in_directory(std::shared_ptr<std::string> directory) {
    using std::strcmp;

    std::shared_ptr<std::vector<std::shared_ptr<std::string>>> result = std::make_shared<std::vector<std::shared_ptr<std::string>>>();
    DIR *dir = opendir(directory->c_str());
    if (dir == NULL) return result;
    dirent *dp;
    while ((dp = readdir(dir)) != nullptr) {
        //Copy the name of the file into a std::string and add it to the vector
        if (strcmp(".", dp->d_name) != 0 && strcmp("..", dp->d_name) != 0) {
            result->push_back(std::make_shared<std::string>(dp->d_name));
        }
    }
    return result;
}

std::shared_ptr<std::vector<std::shared_ptr<std::string>>> FileSystemUtil::get_subdirs_in_directory(std::string directory) {
  using std::strcmp;

  std::shared_ptr<std::vector<std::shared_ptr<std::string>>> result = std::make_shared<std::vector<std::shared_ptr<std::string>>>();
  DIR *dir = opendir(directory.c_str());
  if (dir == NULL) return result;
  dirent *dp;
  while ((dp = readdir(dir)) != nullptr) {
    //Copy the name of the subdir into a std::string and add it to the vector
    if (strcmp(".", dp->d_name) != 0 && strcmp("..", dp->d_name) != 0) {
      std::string fs_obj = directory;
      join_append(fs_obj, dp->d_name); // create new absolute path from input root
      if (directory_exists(fs_obj.c_str())) result->push_back(std::make_shared<std::string>(fs_obj));
    }
  }
  return result;
}

void fulfil::utils::FileSystemUtil::join_append(std::shared_ptr<std::string> base_dir, const std::string &to_append)
{
  if (base_dir->back() == '/')
    *base_dir = to_append.front() == '/' ? base_dir->append(&to_append[1]) : base_dir->append(to_append);
  else
    *base_dir = to_append.front() == '/' ? base_dir->append(to_append) : (base_dir->append("/")).append(to_append);
}

void fulfil::utils::FileSystemUtil::join_append(std::string &base_dir, const std::string &to_append)
{
  if (base_dir.back() == '/')
    base_dir = to_append.front() == '/' ? base_dir.append(&to_append[1]) : base_dir.append(to_append);
  else
    base_dir = to_append.front() == '/' ? base_dir.append(to_append) : (base_dir.append("/")).append(to_append);
}

void FileSystemUtil::get_files_in_directory_helper(std::shared_ptr<std::string> directory,
        std::shared_ptr<std::vector<std::shared_ptr<std::string>>> recursive_results,
        int offset, bool include_dirnames)
{
  using std::strcmp;
  DIR *dir = opendir(directory->c_str());
  if (dir == NULL) return;
  dirent *dp;
  while ((dp = readdir(dir)) != nullptr) {
    //Copy the name of the file into a std::string and aff it to the vector
    if (strcmp(".", dp->d_name) != 0 && strcmp("..", dp->d_name) != 0) {
      std::string fs_obj = *directory;
      join_append(fs_obj, dp->d_name); // create new absolute path from input root
      std::shared_ptr<std::string> fs_obj_ptr = std::make_shared<std::string>(fs_obj);
      std::shared_ptr<std::string> new_entry = std::make_shared<std::string>(&fs_obj[offset]);//make_shared<std::string>(fs_obj.substr(offset));
      if (directory_exists(fs_obj.c_str())) {
        if (include_dirnames && !new_entry->empty()) recursive_results->push_back(new_entry);
        //TODO replace with while loop or risk overflow
        get_files_in_directory_helper(fs_obj_ptr, recursive_results, offset, include_dirnames);
      } else{
        recursive_results->push_back(new_entry);
      }
    }
  }
  closedir(dir);
}

void FileSystemUtil::get_files_in_directory(std::shared_ptr<std::string> directory,
                                            std::shared_ptr<std::vector<std::shared_ptr<std::string>>> recursive_results,
                                            bool relative_path, bool include_dirnames)
{
  int offset = 0;
  if (relative_path) {
    offset = directory->length();
    if (directory->back() != '/') offset++;
  }
  get_files_in_directory_helper(std::move(directory), std::move(recursive_results), offset, include_dirnames);
}

void FileSystemUtil::print_files_in_directory(std::shared_ptr<std::vector<std::shared_ptr<std::string>>> directory_ptr){
  std::vector<std::shared_ptr<std::string>> files = *directory_ptr;
  std::cout << "Directory Contents:" << std::endl;
  for (auto it = files.begin() ; it != files.end(); ++it){
    std::cout << **it << std::endl;
  }
  std::cout << "End of directory." << std::endl;
}

void FileSystemUtil::write_flags_to_file(std::shared_ptr<std::vector<bool>> flags,
                                         std::shared_ptr<std::string> filename)
{
  bool* writable_flags = (bool*) malloc(sizeof(bool) * flags->size());
  for(int i = 0; i < flags->size(); i++)
  {
    writable_flags[i] = flags->at(i);
  }
  long size = flags->size();
  char* content = (char*) malloc(sizeof(long) + sizeof(bool) * size);
  std::memcpy(content, &size, sizeof(long));
  std::memcpy(&content[2], writable_flags, size * sizeof(bool));
  write_to_file(content, sizeof(long) + sizeof(bool) * size, filename->c_str());
  free(content);
}

std::shared_ptr<std::vector<bool>> FileSystemUtil::read_flags_from_file(std::shared_ptr<std::string> filename)
{
  std::shared_ptr<std::string> contents = get_string_from_file(filename->c_str());
  long size = (long)(contents->c_str()[0]);
  bool *flags = (bool *) malloc(sizeof(bool) * size);
  memcpy(flags, &contents->c_str()[2], sizeof(bool) * size);
  std::shared_ptr<std::vector<bool>> shared_flags = std::make_shared<std::vector<bool>>();
  for (int i = 0; i < size; i++)
  {
    shared_flags->push_back(flags[i]);
  }
  return shared_flags;
}

void FileSystemUtil::remove_directory(const std::shared_ptr<std::string>& directory, bool unsafe_recurse)
{
  if(directory_exists(directory->c_str()))
  {
    if (unsafe_recurse)
    {
      std::string command;
      command.append("rm -r ");
      command.append(*directory);
      system(command.c_str());
    } else {
      std::string command;
      command.append("rmdir ");
      command.append(*directory);
      system(command.c_str());
    }
  }
}

void FileSystemUtil::remove_file(const std::string& file_path)
{
      std::string command;
      command.append("rm ");
      command.append(file_path);
      system(command.c_str());
}

