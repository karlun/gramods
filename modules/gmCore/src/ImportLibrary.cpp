
#include <gmCore/ImportLibrary.hh>

#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMCORE;

GM_OFI_DEFINE(ImportLibrary);
GM_OFI_PARAM(ImportLibrary, library, std::string, ImportLibrary::setLibrary);

ImportLibrary::~ImportLibrary() {
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
  this->library = lib;
}

void ImportLibrary::initialize() {
#ifdef WIN32

  handle = GetModuleHandle(library.c_str());

  if (!handle)
    LoadLibrary(library.c_str());

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

  Object::initialize();
}

END_NAMESPACE_GMCORE;
