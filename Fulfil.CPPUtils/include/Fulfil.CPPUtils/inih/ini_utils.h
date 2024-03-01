//
// Created by amber on 2/16/23.
//

#ifndef FULFIL_DISPENSE_INI_UTILS_H
#define FULFIL_DISPENSE_INI_UTILS_H
#include "Fulfil.CPPUtils/inih/INIReader.h"
#include <string>
#include <utility>
#include <vector>

namespace fulfil::utils::ini {

  // TODO move to own helper file
  template<typename T>
  using unmodified_type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

  // Partial specialization over vector
  template <typename V> struct is_vector : std::false_type {};
  template <typename T,typename A> struct is_vector< std::vector<T,A> > : std::true_type {};
  template <typename V> inline constexpr bool is_vector_v = is_vector<V>::value;


  template<typename V>
  void fill_vector(const INIReader& reader, const std::string& section, const std::string& key,
      V &container)
  {
    static_assert (!std::is_const<V>::value && is_vector_v<V>, "Input to fill vector must be a non const vector!");
    if constexpr (std::is_floating_point_v<typename V::
                                 value_type> && !std::is_same<typename V::value_type, double>::value) {
      reader.FillFloatVector(section, key, container);

    } else if constexpr (std::is_same<typename V::value_type, double>::value) {
      reader.FillDoubleVector(section, key, container);
    } else if constexpr(std::is_convertible<typename V::value_type, std::basic_string<char>>::value) {
      reader.FillStringVector(section, key, container);
    } else if constexpr(std::is_integral_v<typename V::value_type> && !std::is_same<bool,typename V::value_type>::value) {
      reader.FillIntegerVector(section, key, container);
    }
  }




  template<typename T>
  auto get_value(const INIReader& reader, const std::string& section, const std::string& key, const T& default_value) {
      if constexpr (std::is_integral_v<T> && !std::is_same<bool,T>::value) {
        return reader.GetInteger(section, key, default_value);
      } else if constexpr (std::is_floating_point_v<T>) {
        return reader.GetFloat(section, key, default_value);
      } else if constexpr (std::is_same<bool, T>::value) {
        return reader.GetBoolean(section, key, default_value);
      } else if constexpr(std::is_convertible<T, std::basic_string<char>>::value) {
        return reader.Get(section, key, default_value);
      } else if constexpr(is_vector_v<T>) {
        if constexpr(std::is_integral_v<typename T::value_type>) {
          return reader.GetIntegerVector(section, key, default_value);
        } else if constexpr(std::is_floating_point_v<typename T::value_type>) {
          return reader.GetFloatVector(section, key, default_value);
        }
      }
  }



  template<typename T>
  auto at(const INIReader& reader, const std::string& section, const char* key) {
    if constexpr (std::is_integral<T>::value && !std::is_same<bool,T>::value) {
      return reader.GetInteger(section, key);
    } else if constexpr (std::is_floating_point<T>::value) {
      return reader.GetFloat(section, key);
    } else if constexpr (std::is_same<bool, T>::value) {
      return reader.GetBoolean(section, key);
    } else if constexpr(std::is_convertible<T, std::basic_string<char>>::value){
      return reader.Get(section, key);
    } else if constexpr(is_vector_v<T>) {
      if constexpr(std::is_integral_v<typename T::value_type>) { return reader.GetIntegerVector(section, key); }
      else if constexpr(std::is_floating_point_v<typename T::value_type>
          && !std::is_same<unmodified_type<typename T::value_type>, double>::value) { return reader.GetFloatVector(section, key); }
      else if constexpr(std::is_same<unmodified_type<typename T::value_type>, double>::value) { return reader.GetDoubleVector(section, key); }
    }
  }

  template<typename LogOp>
  int validate_ini_parse(const INIReader& ini, std::string_view parse_id, LogOp log_fn)
  {
    if (ini.ParseError() < 0) {
      log_fn(parse_id);
      throw std::runtime_error("Failure to parse ini file...");
    }
    return ini.ParseError();
  }



  struct IniSectionReader {
    const INIReader& m_ini_reader;
    std::string m_section_name{};
    template<typename T>
    [[nodiscard]] auto get_value(const std::string& key, const T& default_value) const {
      return fulfil::utils::ini::get_value(m_ini_reader, m_section_name, key, default_value);
    }
    template<typename T>
    [[nodiscard]] auto get_value(std::string section, const std::string& key, const T& default_value) const {
      return fulfil::utils::ini::get_value(m_ini_reader, std::move(section), key, default_value);
    }
    template<typename T>
    [[nodiscard]] auto at(const char* key) const {
      return fulfil::utils::ini::at<T>(m_ini_reader, m_section_name, key);
    }
    template<typename T>
    [[nodiscard]] auto at(std::string section, const char* key) const {
      return fulfil::utils::ini::at<T>(m_ini_reader, std::move(section), key);
    }
    template<typename V>
    void fill_vector(const char* key, V &default_value) const{
      fulfil::utils::ini::fill_vector(m_ini_reader, m_section_name, key, default_value);
    }
    template<typename V>
    void fill_vector(std::string section, const std::string& key, V &default_value) const{
      fulfil::utils::ini::fill_vector(m_ini_reader, std::move(section), key, default_value);
    }


    IniSectionReader(const INIReader&  ini_reader, std::string section_name);
    void update_section(const std::string& section_name);
    [[nodiscard]] std::string get_section() const;

    // make getter function for default initializer ? or throw?

  };

  inline IniSectionReader::IniSectionReader(const INIReader& ini_reader,
      std::string section_name) : m_ini_reader(ini_reader), m_section_name(std::move(section_name)) {};

  inline void IniSectionReader::update_section(const std::string& section_name){
    this->m_section_name = section_name;
  }

  inline std::string IniSectionReader::get_section() const{
    return this->m_section_name;
  }

}
#endif// FULFIL_DISPENSE_INI_UTILS_H
