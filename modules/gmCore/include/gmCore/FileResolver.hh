
#ifndef GRAMODS_CORE_FILERESOLVER
#define GRAMODS_CORE_FILERESOLVER

#include <gmCore/config.hh>

#include <filesystem>

BEGIN_NAMESPACE_GMCORE;

/**
   The FileResolver provides a means to locate resources in a portable
   manner.

   The default behavior (FileResolver::getDefault()) is to first check
   if there is an environment variable GM_URN_FILE pointing at a
   file. If not it checks if there is an environment variable GM_HOME
   that points at a folder with a file "gramods.urn". If not it checks
   if there is a file gramods.urn in the same folder as the executable
   currently running.

   The URN file is parsed for mappings in the form `urn:key:
   path/`. If the path is absolute, then the occurance of "urn:key:"
   is replaced with this path and if it is relative, the key is
   replaced with a path relative the location of the URN file.
*/
class FileResolver {

private:
  FileResolver();
  virtual ~FileResolver();

public:
  /**
     Reads in mappings from the specified URN file.
  */
  bool readUrnFile(std::filesystem::path urn_file);

  /**
     Resolves the specified string as a path using the currently
     loaded rules.
  */
  std::filesystem::path resolve(std::string);

  /**
     Returns the default resolver.
  */
  static FileResolver * getDefault() {
    static FileResolver default_resolver;
    return &default_resolver;
  }

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMCORE;

#endif
