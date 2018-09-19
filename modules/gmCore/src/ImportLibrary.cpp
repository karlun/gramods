
#include <gmCore/ImportLibrary.hh>

#include <dlfcn.h>

BEGIN_NAMESPACE_GMCORE;

GM_OFI_DEFINE(ImportLibrary);
GM_OFI_PARAM(ImportLibrary, lib, std::string, ImportLibrary::setLib);

ImportLibrary::ImportLibrary()
  : library_loaded(false),
    handle(nullptr) {}

ImportLibrary::~ImportLibrary() {
  if (handle)
    dlclose(handle);
  handle = nullptr;
}

void ImportLibrary::setLib(std::string lib) {
  this->lib = lib;
}

void ImportLibrary::initialize() {
  handle = dlopen(lib.c_str(), RTLD_NOW);
  if (handle) {
    library_loaded = true;
  } else {
    char *msg = dlerror();
    GM_ERR("ImportLibrary", "Could not import library '" << lib.c_str() << "':" << msg);
  }
  Object::initialize();
}

END_NAMESPACE_GMCORE;
