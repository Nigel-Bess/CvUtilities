// Read an INI file into easy-to-access name/value pairs.

// inih and INIReader are released under the New BSD license (see LICENSE.txt).
// Go to the project home page for more info:
//
// https://github.com/benhoyt/inih
/* inih -- simple .INI file parser

inih is released under the New BSD license (see LICENSE.txt). Go to the project
home page for more info:

https://github.com/benhoyt/inih

 Code customized by Amber Thomas, Fulfil.ai
*/

#ifndef __INI_H__
#define __INI_H__

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* Typedef for prototype of handler function. */
typedef int (*ini_handler)(void* user, const char* section,
                           const char* name, const char* value);

/* Typedef for prototype of fgets-style reader function. */
typedef char* (*ini_reader)(char* str, int num, void* stream);

/* Parse given INI-style file. May have [section]s, name=value pairs
   (whitespace stripped), and comments starting with ';' (semicolon). Section
   is "" if name=value pair parsed before any section heading. name:value
   pairs are also supported as a concession to Python's configparser.

   For each name=value pair parsed, call handler function with given user
   pointer as well as section, name, and value (data only valid for duration
   of handler call). Handler should return nonzero on success, zero on error.

   Returns 0 on success, line number of first error on parse error (doesn't
   stop on first error), -1 on file open error, or -2 on memory allocation
   error (only when INI_USE_STACK is zero).
*/
int ini_parse(const char* filename, ini_handler handler, void* user);

/* Same as ini_parse(), but takes a FILE* instead of filename. This doesn't
   close the file when it's finished -- the caller must do that. */
int ini_parse_file(FILE* file, ini_handler handler, void* user);

/* Same as ini_parse(), but takes an ini_reader function pointer instead of
   filename. Used for implementing custom or string-based I/O. */
int ini_parse_stream(ini_reader reader, void* stream, ini_handler handler,
                     void* user);

/* Nonzero to allow multi-line value parsing, in the style of Python's
   configparser. If allowed, ini_parse() will call the handler with the same
   name for each subsequent line parsed. */
#ifndef INI_ALLOW_MULTILINE
#define INI_ALLOW_MULTILINE 1
#endif

/* Nonzero to allow a UTF-8 BOM sequence (0xEF 0xBB 0xBF) at the start of
   the file. See http://code.google.com/p/inih/issues/detail?id=21 */
#ifndef INI_ALLOW_BOM
#define INI_ALLOW_BOM 1
#endif

/* Nonzero to allow inline comments (with valid inline comment characters
   specified by INI_INLINE_COMMENT_PREFIXES). Set to 0 to turn off and match
   Python 3.2+ configparser behaviour. */
#ifndef INI_ALLOW_INLINE_COMMENTS
#define INI_ALLOW_INLINE_COMMENTS 1
#endif
#ifndef INI_INLINE_COMMENT_PREFIXES
#define INI_INLINE_COMMENT_PREFIXES ";"
#endif

/* Nonzero to use stack, zero to use heap (malloc/free). */
#ifndef INI_USE_STACK
#define INI_USE_STACK 1
#endif

/* Stop parsing on first error (default is to keep parsing). */
#ifndef INI_STOP_ON_FIRST_ERROR
#define INI_STOP_ON_FIRST_ERROR 0
#endif

/* Maximum line length for any line in INI file. */
#ifndef INI_MAX_LINE
#define INI_MAX_LINE 200
#endif

#ifdef __cplusplus
}
#endif

/* inih -- simple .INI file parser

inih is released under the New BSD license (see LICENSE.txt). Go to the project
home page for more info:

https://github.com/benhoyt/inih

*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#if !INI_USE_STACK
#include <stdlib.h>
#endif

#define MAX_SECTION 50
#define MAX_NAME 50

/* Strip whitespace chars off end of given string, in place. Return s. */
inline static char* rstrip(char* s)
{
  char* p = s + strlen(s);
  while (p > s && isspace((unsigned char)(*--p)))
    *p = '\0';
  return s;
}

/* Return pointer to first non-whitespace char in given string. */
inline static char* lskip(const char* s)
{
  while (*s && isspace((unsigned char)(*s)))
    s++;
  return (char*)s;
}

/* Return pointer to first char (of chars) or inline comment in given string,
   or pointer to null at end of string if neither found. Inline comment must
   be prefixed by a whitespace character to register as a comment. */
inline static char* find_chars_or_comment(const char* s, const char* chars)
{
#if INI_ALLOW_INLINE_COMMENTS
  int was_space = 0;
  while (*s && (!chars || !strchr(chars, *s)) &&
         !(was_space && strchr(INI_INLINE_COMMENT_PREFIXES, *s))) {
    was_space = isspace((unsigned char)(*s));
    s++;
  }
#else
  while (*s && (!chars || !strchr(chars, *s))) {
        s++;
    }
#endif
  return (char*)s;
}

/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
inline static char* strncpy0(char* dest, const char* src, size_t size)
{
  strncpy(dest, src, size);
  dest[size - 1] = '\0';
  return dest;
}

/* See documentation in header file. */
inline int ini_parse_stream(ini_reader reader, void* stream, ini_handler handler,
                            void* user)
{
  /* Uses a fair bit of stack (use heap instead if you need to) */
#if INI_USE_STACK
  char line[INI_MAX_LINE];
#else
  char* line;
#endif
  char section[MAX_SECTION] = "";
  char prev_name[MAX_NAME] = "";

  char* start;
  char* end;
  char* name;
  char* value;
  int lineno = 0;
  int error = 0;

#if !INI_USE_STACK
  line = (char*)malloc(INI_MAX_LINE);
    if (!line) {
        return -2;
    }
#endif

  /* Scan through stream line by line */
  while (reader(line, INI_MAX_LINE, stream) != NULL) {
    lineno++;

    start = line;
#if INI_ALLOW_BOM
    if (lineno == 1 && (unsigned char)start[0] == 0xEF &&
        (unsigned char)start[1] == 0xBB &&
        (unsigned char)start[2] == 0xBF) {
      start += 3;
    }
#endif
    start = lskip(rstrip(start));

    if (*start == ';' || *start == '#') {
      /* Per Python configparser, allow both ; and # comments at the
         start of a line */
    }
#if INI_ALLOW_MULTILINE
    else if (*prev_name && *start && start > line) {

#if INI_ALLOW_INLINE_COMMENTS
      end = find_chars_or_comment(start, NULL);
      if (*end)
        *end = '\0';
      rstrip(start);
#endif

      /* Non-blank line with leading whitespace, treat as continuation
         of previous name's value (as per Python configparser). */
      if (!handler(user, section, prev_name, start) && !error)
        error = lineno;
    }
#endif
    else if (*start == '[') {
      /* A "[section]" line */
      end = find_chars_or_comment(start + 1, "]");
      if (*end == ']') {
        *end = '\0';
        strncpy0(section, start + 1, sizeof(section));
        *prev_name = '\0';
      }
      else if (!error) {
        /* No ']' found on section line */
        error = lineno;
      }
    }
    else if (*start) {
      /* Not a comment, must be a name[=:]value pair */
      end = find_chars_or_comment(start, "=:");
      if (*end == '=' || *end == ':') {
        *end = '\0';
        name = rstrip(start);
        value = lskip(end + 1);
#if INI_ALLOW_INLINE_COMMENTS
        end = find_chars_or_comment(value, NULL);
        if (*end)
          *end = '\0';
#endif
        rstrip(value);

        /* Valid name[=:]value pair found, call handler */
        strncpy0(prev_name, name, sizeof(prev_name));
        if (!handler(user, section, name, value) && !error)
          error = lineno;
      }
      else if (!error) {
        /* No '=' or ':' found on name[=:]value line */
        error = lineno;
      }
    }

#if INI_STOP_ON_FIRST_ERROR
    if (error)
            break;
#endif
  }

#if !INI_USE_STACK
  free(line);
#endif

  return error;
}

/* See documentation in header file. */
inline int ini_parse_file(FILE* file, ini_handler handler, void* user)
{
  return ini_parse_stream((ini_reader)fgets, file, handler, user);
}

/* See documentation in header file. */
inline int ini_parse(const char* filename, ini_handler handler, void* user)
{
  FILE* file;
  int error;

  file = fopen(filename, "r");
  if (!file)
    return -1;
  error = ini_parse_file(file, handler, user);
  fclose(file);
  return error;
}

#endif /* __INI_H__ */


#ifndef __INIREADER_H__
#define __INIREADER_H__


#include <map>
#include <set>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <ctime>
#include <iomanip>

// Read an INI file into easy-to-access name/value pairs. (Note that I've gone
// for simplicity here rather than speed, but it should be pretty decent.)
class INIReader
{
public:
    // Empty Constructor
    INIReader() {}

    // Construct INIReader and parse given filename. See ini.h for more info
    // about the parsing. if use_compiled_default_dir_prefix is true it will prefix
    // the given file name with the directory defined by the DEFAULT_FULFIL_INI_DIR
    // (set during CMake)
    INIReader(std::string filename, bool use_compiled_default_dir_prefix=false, bool throw_on_parse_fail=false);

    // Construct INIReader and parse given file. See ini.h for more info
    // about the parsing.
    INIReader(FILE *file);

    // Return the result of ini_parse(), i.e., 0 on success, line number of
    // first error on parse error, or -1 on file open error.
    int ParseError() const;

    // Return the list of sections found in ini file
    const std::set<std::string>& Sections() const;

    // Get a string value from INI file, returning default_value if not found.
    std::string Get(std::string section, std::string name, const std::string &default_value) const;
    std::string Get(std::string section, std::string name) const;

    // Get an integer (long) value from INI file, returning default_value if
    // not found or not a valid integer (decimal "1234", "-1234", or hex "0x4d2").
    int GetInteger(std::string section, std::string name, int default_value) const;
    int GetInteger(std::string section, std::string name) const;

    // Get a real (floating point double) value from INI file, returning
    // default_value if not found or not a valid floating point value
    // according to strtod().
    double GetReal(std::string section, std::string name, double default_value) const;

    // Get a single precision floating point number value from INI file, returning
    // default_value if not found or not a valid floating point value
    // according to strtof().
    float GetFloat(std::string section, std::string name, float default_value) const;
    float GetFloat(std::string section, std::string name) const;

    // Get a boolean value from INI file, returning default_value if not found or if
    // not a valid true/false value. Valid true values are "true", "yes", "on", "1",
    // and valid false values are "false", "no", "off", "0" (not case sensitive).
    bool GetBoolean(std::string section, std::string name, bool default_value) const;
    bool GetBoolean(std::string section, std::string name) const;


    std::vector<int> GetIntegerVector(std::string section, std::string name) const;
    std::vector<int> GetIntegerVector(std::string section, std::string name, const std::vector<int> &default_value) const;

    std::vector<float> GetFloatVector(std::string section, std::string name) const;
    std::vector<std::string> GetSpaceSepStrVector(std::string section, std::string name) const;

    std::vector<double> GetDoubleVector(const std::string& section, const std::string& name) const;
    void FillFloatVector(std::string section, std::string name, std::vector<float>& float_list) const;
    void FillStringVector(std::string section, std::string name, std::vector<std::string>& string_list) const;

    void set_default_section(std::string default_section);

    std::string get_default_section();

    static std::string get_compiled_default_dir_prefix();

    int copySectionValuesToIniFile(const std::string& filename, std::string section_name,
                                    const std::vector<std::string>& value_names,
                                    bool append_to_file = true, bool rollback_on_error=false)
    {

        std::string write_file;
        if (rollback_on_error) {
            std::string::size_type split = filename.find_last_of("/\\");
            write_file = filename.substr(0,split+1) + "." + filename.substr(split+1) + ".tmp";

        } else
            write_file = filename;
        if (append_to_file && rollback_on_error) {
            this->copy_file(filename, write_file);
        }
        addSectionToIniFile(write_file, section_name, append_to_file);
        std::string fail = "copySectionValuesToIniFile_try_fail";
        int errors = 0;
        for (auto const& vname : value_names){
            std::string val = this->Get(section_name, vname, fail);
            if (val == fail) {
                if (rollback_on_error) {
                    remove(write_file.c_str());
                    return -1;
                }
                errors ++;
            } else
                addValueToIniFile(write_file, vname, val);
        }
        if (rollback_on_error) {
            this->copy_file(write_file, filename);
            remove(write_file.c_str());
        }
        return errors;
    }

    static void addSectionToIniFile(const std::string& filename, const std::string& section_name, bool append_to_file = true)
    {
        std::ofstream file;
        if (append_to_file) {
            file.open(filename, std::ios::app);
            file << "\n";
        }
        else file.open(filename, std::ios::trunc);
        if (section_name.front() != '[')
            file << "["  << section_name;
        if (section_name.back() != ']')
            file << "]";
        file << "\n";

        file.close();
    }

    static void addValueToIniFile(const std::string& filename, const std::string& key, time_t value, bool fmt=true, bool use_local=true)
    {
        std::ofstream file;
        file.open(filename, std::ios::app);
        if (fmt && use_local){
            file << key << " = " << std::put_time(std::localtime(&value), "%F %T %z") << "\n";
        } else if (fmt && !use_local) {
            file << key << " = " << std::put_time(std::gmtime(&value), "%F %T %z") << "\n";
        } else {
            file << key << " = " << value << "\n";
        }

        file.close();
    }

    template<typename T>
    static void addValueToIniFile(const std::string& filename, const std::string& key, T value)
    {
        std::ofstream file;
        file.open(filename, std::ios::app);
        file << key << " = " << value << "\n";
        file.close();
    }

    template<typename T>
    static void addValueToIniFile(const std::string& filename, const std::string& key, std::vector<T> value)
    {
        std::ofstream file;
        file.open(filename, std::ios::app);
        file << key << " =";
        for (const auto val : value)
            file << " " << val;
        file << "\n";
        file.close();
    }

    /**
     * reader1.appendReader(reader2) will add the contents of reader2 to reader1
     * and return 0 on success, -1 if either reader has a -1 (file open error)
     * or line number of first error on parse error (if there is no parse error on reader1, but is on
     * reader2 then the parse error will be num_lines_reader1 + line_num_reader2_error).
     *
     * If there are duplicate sections in reader2, these will update reader1 records and will not
     * increment the num_line count for reader1.
     * */
    int appendReader(const INIReader& new_reader);

    /**
     * Checks to see if specified section::value pair present in ini
     * */
    bool checkForValue(const std::string& section, const std::string& name)
    {
        std::string key = MakeKey(section, name);
        return (this->_values[key].size() > 0);
    }

    void FillDoubleVector(std::string section, std::string name,
                          std::vector<double> &float_list) const;

    void FillIntegerVector(std::string section, std::string name, std::vector<int>& int_list) const;


protected:
    int _error;
    std::map<std::string, std::string> _values;
    std::set<std::string> _sections;
    static std::string MakeKey(std::string section, std::string name);
    static int ValueHandler(void* user, const char* section, const char* name,
                            const char* value);

private:
    std::string _default_section;
    int StrToBoolInt(std::string valstr) const;
    int update_error(const INIReader& new_reader);
    void update_values(const INIReader& new_reader);
    void update_sections(const INIReader& new_reader);
    void copy_file(const std::string& in, const std::string& out)
    {
        std::ofstream file;
        std::ifstream current_buf(in);
        file.open(out, std::ios::trunc);
        file << current_buf.rdbuf();
        file.close();
        current_buf.close();
    }

};

#endif  // __INIREADER_H__


#ifndef __INIREADER__
#define __INIREADER__

#include <algorithm>
#include <iostream>
#include <cctype>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <iterator>

inline INIReader::INIReader(std::string filename, bool use_compiled_default_dir_prefix, bool throw_on_parse_fail)
{
  if (use_compiled_default_dir_prefix){
    std::string prefix = DEFAULT_FULFIL_INI_DIR;
    if (prefix.back() != '/') prefix.append("/");
    filename = (filename.front() == '/') ? prefix.append(&filename[1]) : prefix.append(filename);
  }
  _error = ini_parse(filename.c_str(), ValueHandler, this);
  if (throw_on_parse_fail && _error < 0) {
    throw std::runtime_error("INIReader failed to parse config file " + filename + ". Validate that pathname is correct.");
  }
}

inline INIReader::INIReader(FILE *file)
{
  _error = ini_parse_file(file, ValueHandler, this);
}

inline int INIReader::ParseError() const
{
  return _error;
}

inline std::string INIReader::get_compiled_default_dir_prefix()
{
  return DEFAULT_FULFIL_INI_DIR;
}

inline const std::set<std::string>& INIReader::Sections() const
{
  return _sections;
}

inline std::string INIReader::Get(std::string section, std::string name, const std::string &default_value) const
{
  std::string key = MakeKey(section, name);
  return _values.count(key) ? _values.at(key) : default_value;
}

inline std::string INIReader::Get(std::string section, std::string name) const
{
  std::string key = MakeKey(section, name);
  if  (_values.count(key) == 0) {
    throw std::runtime_error("No valid (non-empty/null) string entry found in file for section ("+ section + ") and name (" + name + ").");
  }
   return  _values.at(key);
}

inline int INIReader::GetInteger(std::string section, std::string name, int default_value) const
{
  std::string valstr = Get(section, name, "");
  const char* value = valstr.c_str();
  char* end;
  // This parses "1234" (decimal) and also "0x4D2" (hex)
  long n = strtol(value, &end, 0);
  return end > value ? n : default_value;
}

inline int INIReader::GetInteger(std::string section, std::string name) const
{
  std::string valstr = Get(section, name, "");
  const char* value = valstr.c_str();
  char* end;
  // This parses "1234" (decimal) and also "0x4D2" (hex)
  long n = strtol(value, &end, 0);
  if (end <= value) throw std::runtime_error("No valid integer entry found in file for section ("+ section + ") and name (" + name + ").");
  return  n;
}

inline double INIReader::GetReal(std::string section, std::string name, double default_value) const
{
  std::string valstr = Get(section, name, "");
  const char* value = valstr.c_str();
  char* end;
  double n = strtod(value, &end);
  return end > value ? n : default_value;
}

inline float INIReader::GetFloat(std::string section, std::string name, float default_value) const
{
  std::string valstr = Get(section, name, "");
  const char* value = valstr.c_str();
  char* end;
  float n = strtof(value, &end);
  return end > value ? n : default_value;
}

inline float INIReader::GetFloat(std::string section, std::string name) const
{
  std::string valstr = Get(section, name);
  const char* value = valstr.c_str();
  char* end;
  float n = strtof(value, &end);
  if (end <= value) throw std::runtime_error("No valid float entry found in file for section ("+ section + ") and name (" + name + ").");
  return n;
}

inline int INIReader::StrToBoolInt(std::string valstr) const
{
  std::transform(valstr.begin(), valstr.end(), valstr.begin(), ::tolower);
  if (valstr == "true" || valstr == "yes" || valstr == "on" || valstr == "1")
    return 1;
  else if (valstr == "false" || valstr == "no" || valstr == "off" || valstr == "0")
    return 0;
  else
    return -1;
}

inline bool INIReader::GetBoolean(std::string section, std::string name, bool default_value) const
{
  std::string valstr = Get(section, name, "");
  // Convert to lower case to make string comparisons case-insensitive
  int value = StrToBoolInt(valstr);
  if (value == -1) {
    return default_value;
  } else {
    return bool(value);
  }
}

inline bool INIReader::GetBoolean(std::string section, std::string name) const
{
  std::string valstr = Get(section, name);
  // Convert to lower case to make string comparisons case-insensitive
  int value = StrToBoolInt(valstr);
  if (value == -1) {
    throw std::runtime_error("No valid bool entry found in file for section ("+ section + ") and name (" + name + ").");
  }
  return bool(value);
}

inline std::string INIReader::MakeKey(std::string section, std::string name)
{
  std::string key = section + "=" + name;
  // Convert to lower case to make section/name lookups case-insensitive
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  return key;
}

inline int INIReader::ValueHandler(void* user, const char* section, const char* name,
                                   const char* value)
{
  INIReader* reader = (INIReader*)user;
  std::string key = MakeKey(section, name);
  if (reader->_values[key].size() > 0)
    reader->_values[key] += "\n";
  reader->_values[key] += value;
  reader->_sections.insert(section);
  return 1;
}
/**
 * Customization
 * TODO if more than a few, should just inherit and extend
 * Should not template due to bools
 * */

inline void INIReader::set_default_section(std::string default_section)
{
  _default_section = default_section;
}

inline std::string INIReader::get_default_section()
{
  return _default_section;
}

inline std::vector<int> INIReader::GetIntegerVector(std::string section, std::string name) const
{
  int ival;
  std::stringstream iss( Get(section, name) );
  std::vector<int> int_list;
  while ( iss >> ival )
    int_list.push_back( ival );
  return int_list;
}

inline std::vector<int> INIReader::GetIntegerVector(std::string section, std::string name, const std::vector<int> &default_value) const
{
  int ival;
  std::string value = Get(section, name, "NOT FOUND");
  if (value == "NOT FOUND") {
    return default_value;
  }
  std::stringstream iss( value );
  std::vector<int> int_list;
  while ( iss >> ival )
    int_list.push_back( ival );
  return int_list;
}

inline std::vector<std::string> INIReader::GetSpaceSepStrVector(std::string section, std::string name) const
{
  if (auto val = Get(section, name, "") ; !val.empty()){
    std::stringstream ss( val );
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    return std::vector<std::string>(begin, end);
  }
  return {};
}

inline std::vector<float> INIReader::GetFloatVector(std::string section, std::string name) const
{
  std::vector<float> float_list{};
  float fval;
  std::istringstream iss( Get(section, name) );
  while ( iss >> fval )
    float_list.push_back( fval );
  return float_list;
}

// TODO once everything is updated to 17, move AWAY from streams
inline std::vector<double> INIReader::GetDoubleVector(const std::string& section, const std::string& name) const
{
  // Since I already had it throwing on error, we can just pop that up the call stack
  // or come back empty
  std::vector<double> double_list{};
  double dval;
  std::istringstream iss( Get(section, name) );
  while ( iss >> dval )
    double_list.push_back( dval );
  return double_list;
}

inline void INIReader::FillDoubleVector(std::string section, std::string name, std::vector<double>& double_list) const
{
  // Since I already had it throwing on error, we can just pop that up the call stack
  // or come back empty
  std::string valstr = Get(section, name, "NO VAL");
  if (valstr != "NO VAL") {
    double dval;
    std::istringstream iss(valstr);
    while (iss >> dval) double_list.push_back(dval);
  }
}

inline void INIReader::FillFloatVector(std::string section, std::string name, std::vector<float>& float_list) const
{
  std::string valstr = Get(section, name, "NO VAL");
  if (valstr != "NO VAL") {
    float fval;
    std::istringstream iss(valstr);
    while (iss >> fval) float_list.push_back(fval);
  }
}

inline void INIReader::FillIntegerVector(std::string section, std::string name, std::vector<int>& int_list) const
{
  std::string valstr = Get(section, name, "NO VAL");
  if (valstr != "NO VAL") {
    int ival;
    std::istringstream iss(valstr);
    while (iss >> ival) int_list.push_back(ival);
  }
}

inline void INIReader::FillStringVector(std::string section, std::string name, std::vector<std::string>& string_list) const
{
  std::string valstr = Get(section, name, "NO VAL");
  const char* value = valstr.c_str();
  if (strcmp(value, "NO VAL") != 0){
    string_list.clear();
    std::string val;
    std::istringstream iss( valstr );
    while ( iss >> val )
      string_list.push_back( val );
  }
}

inline int INIReader::appendReader(const INIReader& new_reader)
{
    this->update_values(new_reader);
    this->update_sections(new_reader);
    int append_error = this->update_error(new_reader);
    this->_error = append_error;
    return this->_error;

}
inline int INIReader::update_error(const INIReader& new_reader)
{
    int current_parse_error = this->ParseError();
    if (current_parse_error != 0) return current_parse_error;
    int new_parse_error = new_reader.ParseError();
    if (new_parse_error <= 0) return new_parse_error;
    return this->_values.size() + new_parse_error;

}

inline void INIReader::update_values(const INIReader& new_reader)
{
    for( auto v : new_reader._values)
    this->_values[v.first] = v.second;
}
inline void INIReader::update_sections(const INIReader& new_reader)
{
    this->_sections.insert(new_reader._sections.begin(), new_reader._sections.end());
}


#endif  // __INIREADER__
