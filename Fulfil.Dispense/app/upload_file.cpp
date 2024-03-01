#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <string>
#include <chrono>
#include <Fulfil.CPPUtils/file_system_util.h>
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include <Fulfil.CPPUtils/curl_control.h>
#include <Fulfil.CPPUtils/logging.h>
#include <iomanip>

using fulfil::utils::Logger;
using fulfil::utils::CurlController;
using fulfil::utils::FileSystemUtil;

bool is_time_dir(const std::string& dir)
{
  std::tm t = {};
  int offset = dir.length() - (dir.back() == '/') + 1;

  std::size_t found = dir.find_last_of('/', offset);
  std::size_t start = found == std::string::npos ? 0 : found +1;
  Logger::Instance()->Trace("Parsing directory node {} for date string.", dir.substr(start, offset));
  std::istringstream ss(dir.substr(start, offset));
  //ss.imbue(std::locale("de_DE.utf-8"));
  ss >> std::get_time(&t, "%m_%d_H%H_M%M_S%S");
  // if stream fails to parse it means it did not follow date format
  return !(ss.fail());
}

/*
 * Note: reserving positive ints for passing back system err codes
 * 'Idle' refers to a filesystem object that has not been modified recently
 * (recently being less than )
 * Return codes:
 *    0: deletable file (should do full upload routine)
 *   -1: empty idle timestamp directory
 *   -2: empty idle non-timestamp directory
 *   -3: non-empty, idle directory
 *   -4: active file system object
 *
 */
int get_handle_code(const std::string& path, uint min_file_idle_time, uint min_directory_idle_time){
  struct stat fileSysObjInfo;
  if (stat(path.c_str(), &fileSysObjInfo) != 0) {  // Use stat( ) to get the info
    Logger::Instance()->Error("Error ({}) reading {} with error: {}", path, strerror(errno));
    return errno;
  }

  std::time_t now_posixt;
  time(&now_posixt); 
  // difference in seconds since filesystem obj was modified,
  std::time_t mod_posixt = now_posixt - fileSysObjInfo.st_mtime;
  if ((fileSysObjInfo.st_mode & S_IFMT) == S_IFDIR) {
    if (mod_posixt < min_directory_idle_time) {
      Logger::Instance()->Debug("Directory {} is too fresh (modified {} seconds ago), "
                               "skipping parse & analysis for now", path , mod_posixt);
      return -4;
    }
    if (FileSystemUtil::directory_empty(path) == 0){
      Logger::Instance()->Trace("Found empty directory:\n\t{}", path);
      if (is_time_dir(path))
        return -1; // Is time dir code.
      return -2; // Is empty idle dir (but not time dir)
    }
    return -3; // Non-empty idle dir
  }
  // if the file was updated recently take no action in case another process is using it
  if (mod_posixt > min_file_idle_time)
    return 0; // deletable file
  // wait on file code
  Logger::Instance()->Debug("File {} is too fresh (modified {} seconds ago), skipping for now", path , mod_posixt);
  return -4;
}


int main()
{
  /**
   * Main path is the same as vid_gen_base_dir
   * */
  // Can define more than one in base_paths of upload_file_config.ini
  Logger* logger = Logger::Instance(Logger::default_logging_dir,"upload_file_logs",Logger::Level::Info,Logger::Level::Info);
  std::vector<std::string> main_paths;

  // Get configs
  std::shared_ptr<INIReader> reader = std::make_shared<INIReader>("upload_file_config.ini", true);
  if (reader->ParseError() < 0) {
    logger->Error("Failure to parse upload_file_config.ini file...");
    exit(EXIT_FAILURE);
  }
  std::string console_log_level = reader->Get("file_info", "console_log_level", "INFO");
  std::string file_log_level = reader->Get("file_info", "file_log_level", "INFO");
  logger->SetConsoleLogLevel(console_log_level);
  logger->SetFileLogLevel(file_log_level);

  std::string base_dir = reader->Get("file_info", "base_path");
  std::string ftp_base_dir = reader->Get("file_info", "ftp_prefix");
  int min_file_idle_time = reader->GetInteger("file_info", "min_file_idle_time", 15);
  int min_directory_idle_time = reader->GetInteger("file_info", "min_directory_idle_time", 120);
  if (min_directory_idle_time < 60) logger->Warn("Configured min_directory_idle_time is {} which is less than a minute "
                                       "(the max amount of time that a video is set to record).", min_directory_idle_time);
  bool move_files = reader->GetBoolean("file_info", "move_uploaded_files", false);
  std::string post_upload_dir;
  if (move_files){
    post_upload_dir = reader->Get("file_info", "post_upload_directory");
    FileSystemUtil::create_nested_directory(std::make_shared<std::string>(post_upload_dir));
  }

  std::shared_ptr<std::string> base_dir_ptr = std::make_shared<std::string>(base_dir);
  std::shared_ptr<std::vector<std::shared_ptr<std::string>>> relative_file_paths = std::make_shared<std::vector<std::shared_ptr<std::string>>>();
  FileSystemUtil::get_files_in_directory(base_dir_ptr, relative_file_paths, true, true);
  if (relative_file_paths->empty()) logger->Warn("Base dir {} was empty, check to make sure this is correct path.", base_dir);
  CurlController curly = CurlController(base_dir, ftp_base_dir);

  for (const auto& path : *relative_file_paths){
    std::string abs_path = base_dir;
    FileSystemUtil::join_append(abs_path, *path);
    int handle_code = get_handle_code(abs_path, min_file_idle_time, min_directory_idle_time);
    Logger::Instance()->Trace("{} handle code is {}", abs_path, handle_code);
    if (handle_code == 0){
      if (move_files) {
        std:: string move_abs_path = post_upload_dir;
        FileSystemUtil::join_append(move_abs_path, *path);
        curly.send_and_move_file(*path, *path, move_abs_path);
      }
      else curly.send_file(*path, *path, true);
    } else if (handle_code == -1) {
      Logger::Instance()->Debug("Removing empty and idle video folder\n\t{}", abs_path);
      FileSystemUtil::remove_directory(std::make_shared<std::string>(abs_path), false);
    }
  }

}



