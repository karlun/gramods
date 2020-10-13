
#include <gmCore/path.hh>

#include <gmCore/Console.hh>
#include <gmCore/FileResolver.hh>
#include <gmCore/RuntimeException.hh>

BEGIN_NAMESPACE_GRAMODS;

std::istream &operator>>(std::istream &in, std::filesystem::path &path) {

  std::string str_path;
  in >> str_path;

  try {
    path = gmCore::FileResolver::getDefault()->resolve(str_path);
  } catch (const gmCore::RuntimeException &e) {
    GM_WRN("std::istream &operator>>(std::istream&, std::filesystem::path&)",
           e.what);
  }

  return in;
}

END_NAMESPACE_GRAMODS;
