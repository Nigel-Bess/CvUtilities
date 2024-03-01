//
// Created by amber on 6/25/20.
//

#include <Fulfil.CPPUtils/file_system_util.h>
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include <Fulfil.CPPUtils/logging.h>

#include <utility>
#include "Fulfil.CPPUtils/curl_control.h"

using fulfil::utils::CurlController;
using fulfil::utils::Logger;
using fulfil::utils::FileSystemUtil;

CurlController::CurlController(std::string current_base_dir, std::string ftp_base_dir)
{
  /* get a curl handle */
  this->curl = curl_easy_init();
  this->current_base_dir = std::move(current_base_dir);
  this->ftp_base_dir = std::move(ftp_base_dir);
  this->retry_limit = 2;

}
CurlController::CurlController(std::string current_base_dir, std::string ftp_base_dir, std::vector<std::string> file_queue)
{
  /* get a curl handle */
  this->curl = curl_easy_init();
  this->file_queue = std::move(file_queue);
  this->current_base_dir = std::move(current_base_dir);
  this->ftp_base_dir = std::move(ftp_base_dir);
  // The retry limit difference is intentional, assume this means user
  // is less likely to be streaming and can afford extra attempts
  this->retry_limit = 4;

}

CurlController::~CurlController()
{
  curl_easy_cleanup(this->curl);
}

void CurlController::bulk_send_files(bool delete_files)
{
  int send_err;
  for(const auto& filename : this->file_queue) {
    send_err = send_file(filename, filename, delete_files);
  }
  this->file_queue.clear();
}
void CurlController::bulk_send_and_move_files(const std::string& new_base_dir)
{
  int send_err;
  for(const auto& filename : this->file_queue){
    std::string new_disk_location = new_base_dir;
    FileSystemUtil::join_append(new_disk_location, filename);
    send_err = send_and_move_file(filename, filename, new_disk_location);

  }
  this->file_queue.clear();
}
int CurlController::send_file(const std::string& local_file, const std::string& ft_output_file, bool delete_files)
{
  std::string full_ftp_path = this->ftp_base_dir;
  FileSystemUtil::join_append(full_ftp_path, ft_output_file);

  std::string local_path;
  int send_err;
  int send_try_counter = 0;
  while (send_try_counter < this->retry_limit) {
    send_err = curl_send_file(local_path, local_file, full_ftp_path);
    if (send_err == 0) break;
    Logger::Instance()->Warn("Failed uploading {}, attempt {}, err code {} in send_file",
            full_ftp_path, send_try_counter, send_err);
    send_try_counter++;
  }
  if (send_err != 0) {
    Logger::Instance()->Error("Issue uploading {}, err code {} in send_file. Dropping frame from stream.",
            full_ftp_path, send_err);

    return send_err;
  }

  if (delete_files && remove( local_path.c_str()) != 0 ) {
    Logger::Instance()->Error("Error deleting uploaded file {} with code {}", local_path, strerror(errno));
    return errno;
  }
  return 0;
}

int CurlController::send_and_move_file(const std::string& local_file, const std::string&  ft_output_file, const std::string& new_disk_location)
{
  std::string full_ftp_path = this->ftp_base_dir;
  FileSystemUtil::join_append(full_ftp_path, ft_output_file);

  std::string local_path;
  int send_err;
  int send_try_counter = 0;
  while (send_try_counter < this->retry_limit) {
    send_err = curl_send_file(local_path, local_file, full_ftp_path);
    if (send_err == 0) break;
    Logger::Instance()->Warn("Failed uploading {}, attempt {}, err code {} in send_and_move_file",
                             full_ftp_path, send_try_counter, send_err);
    send_try_counter++;
  }
  if (send_err != 0) {
    Logger::Instance()->Error("Issue uploading {}, err code {} in send_and_move_file. Dropping frame from stream.",
                              full_ftp_path, send_err);
    return send_err;
  }

  // Move files
  FileSystemUtil::make_containing_directory(std::make_shared<std::string>(new_disk_location));
  if(rename(local_path.c_str(), new_disk_location.c_str()) != 0)
  {
    Logger::Instance()->Error("Error moving uploaded file {}\n\tto new location {}\n\twith error {}",
            local_path, new_disk_location, strerror(errno));
    return errno;
  }
  return 0;
}
void CurlController::add_file_task(const std::string& file)
{
  this->file_queue.push_back(file);
}
void CurlController::bulk_add_file_tasks(std::vector<std::string> bulk_files)
{
  this->file_queue.reserve(this->file_queue.size() + bulk_files.size());
  this->file_queue.insert(this->file_queue.end(), bulk_files.begin(), bulk_files.end());
}

int CurlController::curl_send_file(std::string& local_path, const std::string& local_filename, const std::string& ft_output_file)
{
  // Get starting timepoint
  CURLcode res;
  FILE* hd_src;
  struct stat file_info;
  curl_off_t fsize;

  local_path = this->current_base_dir;

  FileSystemUtil::join_append(local_path, local_filename);
  const char* local_file = local_path.c_str();
  auto start = std::chrono::high_resolution_clock::now();

  // get the file size of the local file //
  if (stat(local_file, &(file_info))) {
    printf("Couldn't open '%s': %s\n", local_file, strerror(errno));
  }
  //TODO use for verification
  fsize = (curl_off_t) file_info.st_size;

  // get a FILE * of the same file
  hd_src = fopen(local_file, "rb");

  if (this->curl)
  {

    // we want to use our own read function
    curl_easy_setopt(this->curl, CURLOPT_READFUNCTION, read_callback);

    curl_easy_setopt(this->curl, CURLOPT_FTP_CREATE_MISSING_DIRS,
                     CURLFTP_CREATE_DIR_RETRY);

    // enable uploading
    curl_easy_setopt(this->curl, CURLOPT_UPLOAD, 1L);

    // specify target
    curl_easy_setopt(this->curl, CURLOPT_URL, ft_output_file.c_str());

    // now specify which file to upload
    curl_easy_setopt(this->curl, CURLOPT_READDATA, hd_src);

    curl_easy_setopt(this->curl, CURLOPT_TIMEOUT, 5L); //timeout if not completed in 5 seconds

    try
    {
      // Now run off and do what you've been told!
      res = curl_easy_perform(this->curl);

      // Check for errors
      if (res != CURLE_OK)
      {
        //TODO better handling
        fclose(hd_src); /* close the local file */
        return res;
      }

    }
    catch (const std::runtime_error &e)
    {
      //TODO better handling
      fclose(hd_src); /* close the local file */
      Logger::Instance()->Error("Error was thrown during curl send: {}", e.what());
      return -1;
    }


  }
  fclose(hd_src); /* close the local file */
  return 0;
}

size_t CurlController::read_callback(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  curl_off_t nread;
  /* in real-world cases, this would probably get this data differently
     as this fread() stuff is exactly what the library already would do
     by default internally */
  size_t retcode = fread(ptr, size, nmemb, stream);

  nread = (curl_off_t)retcode;

  //fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T" bytes from file\n", nread);
  return retcode;
}