//
// Created by amber on 6/25/20.
//

#ifndef FULFIL_CPPUTILS_CURL_CONTROLLER_H
#define FULFIL_CPPUTILS_CURL_CONTROLLER_H


#include <cstdio>
#include <cstring>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include <iostream>
#include <dirent.h>
#include <chrono>
#include <vector>



namespace fulfil {
    namespace utils {
/**
 * Controls file upload process to ftp server, moving and deleting
 * */
        class CurlController {
        public:
            // stuff
            CurlController(std::string current_base_dir, std::string ftp_base_dir);
            CurlController(std::string current_base_dir, std::string ftp_base_dir, std::vector<std::string> file_queue);
            ~CurlController();

            void bulk_send_files(bool delete_files);
            void bulk_send_and_move_files(const std::string& new_base_dir);
            int send_file(const std::string& local_file, const std::string&  ft_output_file, bool delete_files);
            int send_and_move_file(const std::string& local_file, const std::string&  ft_output_file, const std::string&  local_output_file);
            void add_file_task(const std::string& file);
            void bulk_add_file_tasks(std::vector<std::string> bulk_files);
        private:
            // Objects for curl transfer
            CURL* curl;

            std::string current_base_dir;
            std::string ftp_base_dir;
            std::vector<std::string> file_queue;
            int retry_limit;

            int curl_send_file(std::string& local_path, const std::string& local_filename, const std::string& ft_output_file);

            static size_t read_callback(void *ptr, size_t size, size_t nmemb, FILE *stream);

        };
    }
}

#endif //FULFIL_CPPUTILS_CURL_CONTROLLER_H
