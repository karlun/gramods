
#include <gmCore/ImportLibrary.hh>

#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMCORE;

GM_OFI_DEFINE(ImportLibrary);
GM_OFI_PARAM(ImportLibrary, library, std::string, ImportLibrary::setLibrary);
GM_OFI_PARAM(ImportLibrary, prefix, std::string, ImportLibrary::setPrefix);
GM_OFI_PARAM(ImportLibrary, suffix, std::string, ImportLibrary::setSuffix);

struct ImportLibrary::Impl {

  ~Impl();

  void initialize();

  bool library_loaded;
  std::string library;

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

void ImportLibrary::setLibrary(std::string lib) {
  this->_impl->library = lib;
}

bool ImportLibrary::isLoaded() {
  return _impl->library_loaded;
}

void ImportLibrary::initialize() {
  _impl->initialize();
  Object::initialize();
}

void ImportLibrary::Impl::initialize() {
#ifdef WIN32

  handle = GetModuleHandle(library.c_str());

  if (!handle)
    handle = LoadLibrary(library.c_str());

  if (!handle) {
    TCHAR buffer[255];
    DWORD dw = GetLastError();
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dw, 0, buffer, 255, NULL);
    GM_ERR("ImportLibrary", "" << buffer);
  } else {
    library_loaded = true;
  }

#else

  handle = dlopen(library.c_str(), RTLD_NOW);
  if (handle) {
    library_loaded = true;
  } else {
    char *msg = dlerror();
    GM_ERR("ImportLibrary", "Could not import library '" << library.c_str() << "':" << msg);
  }

#endif
}

void ImportLibrary::setPrefix(std::string str) {
  _impl->prefix = str;
}

void ImportLibrary::setSuffix(std::string str) {
  _impl->suffix = str;
}

END_NAMESPACE_GMCORE;
