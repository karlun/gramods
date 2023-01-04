
#include <gmCore/ImportLibrary.hh>

#include <gmCore/Console.hh>
#include <gmCore/FileResolver.hh>

BEGIN_NAMESPACE_GMCORE;

GM_OFI_DEFINE(ImportLibrary);
GM_OFI_PARAM2(ImportLibrary, libraryFile, std::filesystem::path, setLibraryFile);
GM_OFI_PARAM2(ImportLibrary, library, std::string, setLibrary);
GM_OFI_PARAM2(ImportLibrary, prefix, std::string, setPrefix);
GM_OFI_PARAM2(ImportLibrary, suffix, std::string, setSuffix);
GM_OFI_PARAM2(ImportLibrary, dbgSuffix, std::string, setDbgSuffix);

struct ImportLibrary::Impl {

  ~Impl();

  void initialize();

  bool library_loaded;
  std::filesystem::path library_file;
  std::string library;

#ifdef NDEBUG
  std::string dbg_suffix = "";
#else
  std::string dbg_suffix = "-d";
#endif

#ifdef WIN32

  std::string prefix = "";
  std::string suffix = ".dll";

  HMODULE handle = 0;

#else

#ifdef __APPLE__
  std::string prefix = "lib";
  std::string suffix = ".dylib";
#else
  std::string prefix = "lib";
  std::string suffix = ".so";
#endif

  void *handle = nullptr;

#endif

};

ImportLibrary::ImportLibrary()
  : _impl(std::make_unique<Impl>()) {}

ImportLibrary::~ImportLibrary() {}

ImportLibrary::Impl::~Impl() {
#ifdef WIN32
  if (handle)
    FreeLibrary(handle);
  handle = 0;
#else
  if (handle)
    dlclose(handle);
  handle = nullptr;
#endif
}

void ImportLibrary::setLibraryFile(std::filesystem::path path) {
  _impl->library_file = gmCore::FileResolver::getDefault()->resolve(path);
}

void ImportLibrary::setLibrary(std::string lib) {
  _impl->library = lib;
}

bool ImportLibrary::isLoaded() {
  return _impl->library_loaded;
}

void ImportLibrary::initialize() {
  _impl->initialize();
  Object::initialize();
}

void ImportLibrary::Impl::initialize() {

  if (library_file.empty()) {
    if (!library.empty()) {
      library_file = prefix + library + dbg_suffix + suffix;
    } else {
      GM_ERR("ImportLibrary", "Cannot load library - no library specified");
      return;
    }
  }

#ifdef WIN32

  handle = GetModuleHandle(library_file.u8string().c_str());

  if (!handle)
    handle = LoadLibrary(library_file.u8string().c_str());

  if (!handle) {
    TCHAR buffer[255];
    DWORD dw = GetLastError();
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dw, 0, buffer, 255, NULL);
    GM_ERR("ImportLibrary", "While importing '" << library_file << "': " << buffer);
  } else {
    library_loaded = true;
  }

#else

  handle = dlopen(library_file.c_str(), RTLD_NOW);
  if (handle) {
    library_loaded = true;
  } else {
    char *msg = dlerror();
    GM_ERR("ImportLibrary", "While importing '" << library_file << "': " << msg);
  }

#endif
}

void ImportLibrary::setPrefix(std::string str) {
  _impl->prefix = str;
}

void ImportLibrary::setSuffix(std::string str) {
  _impl->suffix = str;
}

void ImportLibrary::setDbgSuffix(std::string str) {
  _impl->dbg_suffix = str;
}

END_NAMESPACE_GMCORE;
