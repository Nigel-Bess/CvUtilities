//
// Created by amber on 4/14/22.
//

#ifndef FULFIL_DISPENSE_MAKE_MEDIA_H
#define FULFIL_DISPENSE_MAKE_MEDIA_H
#include "Fulfil.DepthCam/data/data_generator.h"
#include <cstring>
#include <ctime>
#include <experimental/filesystem>
#include <string>


namespace make_media::paths {

      const tm *get_time_info();

      std::string make_formatted_datetime_str(std::size_t datetime_buffer_size, const char *fmt);

      std::string get_datetime_str();

      std::string get_day_str();

      bool safe_dir_location(std::experimental::filesystem::path path);

      std::experimental::filesystem::path add_basedir_date_suffix_and_join(const std::string &basedir_to_date, const std::string &subdir);

      template<typename T>
      void insert_timestamp_dir_in_path(std::experimental::filesystem::path &input_dir,
        T&& rest_of_path) {
          static_assert((std::is_convertible_v< T&& , std::experimental::filesystem::path > ),
            "insert_timestamp_dir_in_path requires a type that is trivially convertible to std::filesystem::path");
          input_dir /= get_datetime_str();
          input_dir /= std::forward<T>(rest_of_path);
      }

      template<typename ... Args>
      std::experimental::filesystem::path join_as_path(Args&& ... args) {
        static_assert((std::is_convertible_v< Args&& , std::experimental::filesystem::path > && ...),
          "join_to_path must take type that is trivially convertible to a std::filesystem::path "
          "(i.e basic_string and string_view)");
         { return (std::experimental::filesystem::path {std::forward<Args>(args)} /= ...); }
      }

      //template<typename DataGenBuilderFn>

    }


namespace make_media::frame {
    cv::Mat copy_matrix(const cv::Mat& cv_frame);
    cv::Mat copy_matrix(const std::shared_ptr<cv::Mat>& cv_frame);


    bool write_image(const std::shared_ptr<cv::Mat>& image, std::experimental::filesystem::path image_path);
    bool write_image(const std::shared_ptr<cv::Mat>& image, std::experimental::filesystem::path image_path, float reduction_factor);
    bool is_empty(const std::shared_ptr<cv::Mat>& mat);
    bool is_empty(const cv::Mat& mat);


    template<typename Frame, typename SaveOp>
     bool save_frame(Frame cv_frame, SaveOp save_fn) {
        return bool(save_fn(cv_frame));
    }
}



#endif //FULFIL_DISPENSE_MAKE_MEDIA_H
