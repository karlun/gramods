
#include <gmCore/FileResolver.hh>

#include <gmCore/InvalidArgument.hh>
#include <gmCore/Console.hh>

#include <algorithm>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <cstring>
#include <cstdlib>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <errno.h>
#include <linux/limits.h>
#include <unistd.h>
#endif

namespace {
inline std::string trim(const std::string &s) {
  auto wsfront = std::find_if_not(
      s.begin(), s.end(), [](int c) { return std::isspace(c); });
  auto wsback = std::find_if_not(s.rbegin(), s.rend(), [](int c) {
                  return std::isspace(c);
                }).base();
  return (wsback <= wsfront ? std::string() : std::string(wsfront, wsback));
}
}

BEGIN_NAMESPACE_GMCORE;

struct FileResolver::Impl {
  std::filesystem::path resolve(std::string str_path, size_t recursion);
  std::unordered_map<std::string, std::filesystem::path> map;
};

FileResolver::FileResolver() : _impl(std::make_unique<Impl>()) {

  char *env_urn_file = std::getenv("GM_URN_FILE");
  if (env_urn_file) {

    std::filesystem::path urn_file(env_urn_file);

    if (!std::filesystem::exists(urn_file) ||
        std::filesystem::is_directory(urn_file)) {
      GM_WRN(
          "FileResolver",
          "GM_URN_FILE environment variable exists but does not point at a URN file: '"
              << urn_file << "'");
    } else if (readUrnFile(env_urn_file)) {
      return;
    }
  }

  char *env_root = std::getenv("GM_HOME");
  if (env_root) {

    std::filesystem::path root(env_root);
    std::filesystem::path urn_file = root / "gramods.urn";

    if (!std::filesystem::exists(root)) {
      GM_WRN(
          "FileResolver",
          "GM_HOME environment variable exists but does not point at a valid location: '"
              << root << "'");
    } else if (!std::filesystem::is_directory(root)) {
      GM_WRN("FileResolver",
             "GM_HOME does not point at a folder: '" << root << "'");
    } else if (!std::filesystem::exists(urn_file)) {
      GM_WRN("FileResolver",
             "GM_HOME does not contain a gramods.urn file: '" << urn_file
                                                              << "'");
    } else if (std::filesystem::is_directory(urn_file)) {
      GM_WRN("FileResolver",
             "gramods.urn file is not a valid file: '" << urn_file << "'");
    } else if (readUrnFile(urn_file)) {
      return;
    }
  }

  std::filesystem::path exec_folder = getPathToExecutable().parent_path();
  if (!std::filesystem::is_directory(exec_folder)) {
    GM_WRN(
        "FileResolver",
        "Could not find 'gramods.urn' file (tried $GM_URN_FILE, $GM_HOME and exec folder)");
    return;
  }

  std::filesystem::path urn_file = exec_folder / "gramods.urn";

  if (!std::filesystem::exists(urn_file)) {
    GM_WRN("FileResolver",
           "Exec folder does not contain a gramods.urn file: '" << urn_file
                                                                << "'");
  } else if (std::filesystem::is_directory(urn_file)) {
    GM_WRN("FileResolver",
           "File gramods.urn in exec folder is not a valid file: '" << urn_file
                                                                    << "'");
  } else if (readUrnFile(urn_file)) {
    return;
  }

  GM_WRN(
      "FileResolver",
      "Could not find a readable 'gramods.urn' file (tried $GM_URN_FILE, $GM_HOME and exec folder)");
}

FileResolver::~FileResolver() {}

bool FileResolver::readUrnFile(std::filesystem::path urn_file) {

  std::filesystem::path root = urn_file.parent_path();

  std::ifstream if_urn(urn_file);
  if (!if_urn) {
    GM_WRN("FileResolver", "Could not read urn file '" << urn_file << "'");
    return false;
  }

  std::string line;
  while (std::getline(if_urn, line)) {

    line = trim(line);
    if (line.empty()) continue;

    size_t comment_s_pos = line.find("//");
    size_t comment_h_pos = line.find("#");
    size_t comment_pos = std::min(comment_s_pos, comment_h_pos);
    if (comment_pos != std::string::npos) {
      line = line.substr(0, comment_pos);
      line = trim(line);
      if (line.empty()) continue;
    }

    if (line.size() < 8) {
      GM_WRN("FileResolver",
             "Malformed line in URN file: '"
                 << line
                 << "'."
                    " Must be in form 'urn:mykey: path/to/resource',"
                    " where 'urn' and colons are literal.");
      continue;
    }

    size_t urn_pos = line.find("urn:");
    size_t key_end = line.find(":", 4);
    if (urn_pos != 0 || key_end == std::string::npos) {
      GM_WRN("FileResolver",
             "Malformed line in URN file: '"
                 << line
                 << "'."
                    " Must be in form 'urn:mykey: path/to/resource',"
                    " where 'urn' and colons are literal.");
      continue;
    }

    std::string key = trim(line.substr(4, key_end - 4));
    std::string str_path = trim(line.substr(key_end + 1));

    if (key.empty() || str_path.empty()) {
      GM_WRN("FileResolver",
             "Malformed line in URN file: '"
                 << line << "' (key: '" << key << "', path: '" << str_path
                 << "')."
                 << " Must be in form 'urn:mykey: path/to/resource',"
                 << " where 'urn' and colons are literal.");
      continue;
    }

    std::filesystem::path path(str_path);
    if (path.is_relative())
      path = root / path;

    GM_DBG2("FileResolver", "Adding URN '" << key << "' -> '" << path << "'");
    _impl->map[key] = path;
  }

  return true;
}

std::filesystem::path FileResolver::resolve(std::string str_path, Check check) {

  auto path = _impl->resolve(str_path, 0);

  switch (check) {
  case Check::ReadableFile: {
    std::ifstream fin(path);
    if (!fin) {
      if (str_path == path)
        throw gmCore::InvalidArgument(
            GM_STR("Path " << path << " is not readable"));
      else
        throw gmCore::InvalidArgument(
            GM_STR("Path " << path << " (" << str_path << ") is not readable"));
    }
    if (str_path == path)
      GM_DBG1("FileResolver", "Resolved " << str_path << " as readable file");
    else
      GM_DBG1("FileResolver", "Resolved " << str_path << " into " << path << " as readable file");
  } break;

  case Check::WritableFile: {
    std::ofstream fout(path, std::ofstream::app);
    if (!fout) {
      if (str_path == path)
        throw gmCore::InvalidArgument(
            GM_STR("Path " << path << " is not writable"));
      else
        throw gmCore::InvalidArgument(
            GM_STR("Path " << path << " (" << str_path << ") is not writable"));
    }
    if (str_path == path)
      GM_DBG1("FileResolver", "Resolved " << str_path << " as writable file");
    else
      GM_DBG1("FileResolver", "Resolved " << str_path << " into " << path << " as writable file");
  } break;

  case Check::None:
    if (str_path == path)
      GM_DBG1("FileResolver", "Resolved " << str_path << " without checks");
    else
      GM_DBG1("FileResolver", "Resolved " << str_path << " into " << path << " without checks");
  }

  return path;
}

std::filesystem::path FileResolver::Impl::resolve(std::string str_path, size_t recursion) {

  if (recursion > 10) return std::filesystem::path(str_path);

  if (str_path.empty()) return {};

  if (str_path.find("urn:") == 0) {
    size_t end_pos = str_path.find(":", 4);
    if (end_pos == std::string::npos)
      throw gmCore::InvalidArgument(
          GM_STR("path urn without name: '" << str_path << "'"));

    std::string key = str_path.substr(4, end_pos - 4);
    str_path = (map[key] / str_path.substr(end_pos + 1)).u8string();
    return resolve(str_path, recursion + 1);
  }

  size_t var_pos = str_path.find("${");
  if (var_pos < str_path.size() - 4) {
    size_t var_end = str_path.find("}", var_pos);
    if (var_end == std::string::npos)
      throw gmCore::InvalidArgument
        (GM_STR("path variable without closing bracket: '" << str_path.substr(var_pos) << "'"));

    std::string var_name = str_path.substr(var_pos + 2, var_end - var_pos - 2);
    char * var_value_ptr = std::getenv(var_name.c_str());
    if (!var_value_ptr || strlen(var_value_ptr) > 4096)
      throw gmCore::InvalidArgument(GM_STR("path variable ('" << var_name << "') not of valid value"));

    std::string var_value(var_value_ptr);
    str_path = str_path.substr(0, var_pos) + var_value + str_path.substr(var_end + 1);
    return resolve(str_path, recursion + 1);
  }

  return std::filesystem::path(str_path).make_preferred();
}

std::filesystem::path FileResolver::getPathToExecutable() {

#ifdef _WIN32

  constexpr int PATH_MAX = 32768;
  wchar_t exe_path[PATH_MAX] = {0};

  DWORD num_chars = GetModuleFileNameW(NULL, exe_path, PATH_MAX);

  if (num_chars == 0) {
    GM_WRN("FileResolver", "Failed getting Executable path: " << GetLastError());
    return {};
  }

  return std::filesystem::path(exe_path);

#else

  std::filesystem::path exe_link("/proc/self/exe");
  std::filesystem::path exe_path = std::filesystem::read_symlink(exe_link);
  return std::filesystem::path(exe_path);

#endif
}

END_NAMESPACE_GMCORE;
