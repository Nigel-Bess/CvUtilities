#ifndef FULFIL_CPPUTILS_FILE_SYSTEM_HELPER_H
#define FULFIL_CPPUTILS_FILE_SYSTEM_HELPER_H

#include <memory>
#include <vector>

namespace fulfil
{
namespace utils
{
/**
 * The purpose of this class is to make some common filesystem tasks
 * more easy.
 */
class FileSystemUtil {
public:
    /**
     * Returns whether the given directory exists.
     * @param directory const char pointer of the directory path.
     * @return true if directory exists, false otherwise
     */
    static bool directory_exists(const char *directory);
  /**
   * Returns whether the given directory is empty. Note that this check may be redundant if you
   * are already iterating through a dirent*
   * @param pathname to the directory.
   * @return function returns 0 if empty, -1 if not empty and some number > 0 if error
   */
      static int directory_empty(const char *directory);
      static int directory_empty(const std::string& directory);
      static int directory_empty(const std::shared_ptr<std::string>&directory);


    /**
     * Creates a directory specified by the provided path.
     * @param directory a const char point of the directory path.
     */
    static void create_directory(const char *directory);
    /**
     * Creates a nested directory specified by the provided path.
     * Less safe than create directory because, but should validate that
     * created dir is in the same /home/${user} path.
     * @param directory a const char point of the directory path.
     */
    static void create_nested_directory(const std::shared_ptr<std::string>& directory);
    static void create_nested_directory(const std::string& directory);
    /**
     * Returns whether the file at the given path exists.
     * @param filename const char pointer of the path to the file.
     * @return true if the file exists, false otherwise.
     */
    static bool file_exists(const char *filename);
    /**
     * Writes the given data of the given size to the filename.
     * @param data constant pointer to char array of data (can be thought of as
     * an array of bytes since char is one byte).
     * @param data_size number of bytes of the data to be written.
     * @param filename const pointer to char array with the filename.
     */
    static void write_to_file(const char* data, long data_size, const char* filename);
    /**
     * Reads the data from a file into a string and returns the string.
     * @param filename const pointer to char array of the file path.
     * @return pointer to a string containing the byte data read from the file
     * located at the file path.
     * @throws exception when the file does not exist.
     * @note It is currently unclear whether some data at the tail end of the file
     * will be excluded with the current implementation.
     */
    static std::shared_ptr<std::string> get_string_from_file(const char *filename);
    static std::string get_string_from_file(const std::string& filename);
    /**
     * Creates a directory if no directory in the same location exists.
     * @param dirname const pointer to a char array that contains the directory name.
     */
    static void create_dir_if_not_exist(const char *dirname);
    /**
     * Writes the specified amount of data from the given data to the file located
     * at the given filepath.
     * @param data const pointer to char array (byte array) containing the data to be
     * written.
     * @param data_size number of bytes to write.
     * @param filename const pointer to char array containing the filename.
     */
    static void write_to_file_if_not_exists(const char *data, long data_size, const char *filename);
    /**
     * Fof each of the contents in the directory, returns the name in the filesystem (excludes .. and .).
     * @param directory pointer to the string with the directory path.
     */
    /**
     * For each of the contents in the directory, returns the name from the filesystem (excludes .. for parent
     * directory and . for current directory). Example: say the directory /home/usr/tmp contains a directory named
     * test and a file named details.txt. Calling this function with the directory path /home/usr/tmp would return
     * a vector of two strings, one containing test and the other containing details.txt.
     * @param directory pointer to the string containing directory path.
     * @return pointer to vector of strings representing name of each item in directory.
     * @throws exception if the directory does not exists.
     */
    static std::shared_ptr<std::vector<std::shared_ptr<std::string>>> get_files_in_directory(std::shared_ptr<std::string> directory);

    static void get_files_in_directory(std::shared_ptr<std::string> directory,
                                       std::shared_ptr<std::vector<std::shared_ptr<std::string>>> recursive_results,
                                       bool relative_path = true, bool include_dirnames = false);

    static std::shared_ptr<std::vector<std::shared_ptr<std::string>>> get_subdirs_in_directory(std::string directory);

    /**
     * Appends the value to_append to base_dir reference while ensuring that there is a '/' to
     * create a linux style path.
     */
    static void join_append(std::string &base_dir, const std::string &to_append);
    static void join_append(std::shared_ptr<std::string> base_dir, const std::string &to_append);

    /**
     * Breaks the path into basedir and filename pair, should work for linux or windows styled paths
     */
    static std::pair<std::string, std::string> split_basedir(const std::shared_ptr<std::string>& path_name);
    static std::pair<std::string, std::string> split_basedir(const std::string& path_name);
    static std::string get_extension(const std::string& filename);

    /**
     * Breaks the path into basedir and filename pair, should work for linux or windows styled paths
     */
    static void make_containing_directory(const std::shared_ptr<std::string>& path_name);

    /**
     * Creates timestamp in current localtime in format "%m_%d_H%H_M%M_S%S"
     * Example output for Nov. 12th at 1:16pm (in pst generated in California)-> "11_12_H13_M16_S40"
     * @return shared_ptr to timestamp string
     */
    static std::shared_ptr<std::string> create_datetime_string(bool date_only=false);
    /**
     * Pretty prints the result of get_files_in_directory.
     * @param result of the function get_files_in_directory();
     * @throws exception if pointer does not point to valid object
     */
    static void print_files_in_directory(std::shared_ptr<std::vector<std::shared_ptr<std::string>>> directory_ptr);
    /**
     * Reads a vector of booleans from a file.
     * @param filename pointer to the string containing the file path.
     * @return a vector of booleans parsed from the file.
     * @throws exception if the file does not exist.
     */
    static std::shared_ptr<std::vector<bool>> read_flags_from_file(std::shared_ptr<std::string> filename);
    /**
     * Writes the given vector of booleans to the provided file path.
     * @param flags pointer to vector of booleans.
     * @param filename pointer to string containing the destination file path.
     */
    static void write_flags_to_file(std::shared_ptr<std::vector<bool>> flags, std::shared_ptr<std::string> filename);
    /**
     * Removes the directory at the given filepath.
     * @param directory pointer to string with path to directory.
     */
    static void remove_directory(const std::shared_ptr<std::string>& directory, bool unsafe_recurse=true);
    static void remove_file(const std::string &file_path);


private:
    static void get_files_in_directory_helper(std::shared_ptr<std::string> directory,
                                       std::shared_ptr<std::vector<std::shared_ptr<std::string>>> recursive_results,
                                       int offset, bool include_dirnames);


};
} // namespace utils
} // namespace fulfil

#endif
