//
// Created by amber on 4/14/22.
//

#include "Fulfil.Dispense/visualization/make_media.h"
#include <array>

namespace std_filesystem = std::experimental::filesystem;

const tm* make_media::paths::get_time_info () {
    time_t rawtime{0};
    struct tm * timeinfo{};
    time (&rawtime);
    timeinfo = localtime(&rawtime);
    return timeinfo;
}

std::string make_media::paths::make_formatted_datetime_str(std::size_t datetime_buffer_size, const char* fmt) {
    char time_str_buffer[datetime_buffer_size];
    const tm *timeinfo = get_time_info();
    char* time_buf_ref = &time_str_buffer[0];
    strftime(time_buf_ref, sizeof(time_str_buffer), fmt, timeinfo);
    return {time_buf_ref};
}

std::string make_media::paths::get_datetime_str(){
    constexpr std::size_t buffer_size = 24;
    constexpr const char* datetime_fmt = "%Y_%m_%d_H%H_M%M_S%S";
    return make_formatted_datetime_str(buffer_size, datetime_fmt);
}

std::string make_media::paths::get_day_str(){
    constexpr std::size_t buffer_size = 16;
    constexpr const char* datetime_fmt = "%Y_%m_%d";
    return make_formatted_datetime_str(buffer_size, datetime_fmt);
}

std_filesystem::path make_media::paths::add_basedir_date_suffix_and_join(const std::string& basedir_to_date,
                                                                         const std::string& subdir){
    std::string::size_type path_slice = (basedir_to_date.back() == '/') ? basedir_to_date.size() -1 : basedir_to_date.size();
    std_filesystem::path base_path {basedir_to_date.substr(0,path_slice) + "_" + get_day_str()};
    std_filesystem::create_directories(base_path);
    base_path /= subdir;
    return base_path;
}

bool make_media::paths::safe_dir_location(std_filesystem::path path) {
    auto home = std_filesystem::path{std::getenv("HOME")};
    return (!std_filesystem::is_empty(path) )
           && std::equal(home.begin(), home.end(), path.begin());
}


cv::Mat make_media::frame::copy_matrix(const std::shared_ptr<cv::Mat>& cv_frame) {
    if (cv_frame == nullptr) return cv::Mat{};
    return cv_frame->clone();
}

cv::Mat make_media::frame::copy_matrix(const cv::Mat& cv_frame) {
    return cv_frame.clone();
}

bool make_media::frame::is_empty(const cv::Mat& mat) {
    return mat.empty();
}

bool make_media::frame::is_empty(const std::shared_ptr<cv::Mat>& mat) {
    return (mat == nullptr) || mat->empty();
}
