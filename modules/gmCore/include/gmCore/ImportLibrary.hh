
#ifndef GRAMODS_CORE_IMPORTLIBRARY
#define GRAMODS_CORE_IMPORTLIBRARY

#include <gmCore/config.hh>
#include <gmCore/OFactory.hh>
#include <gmCore/Object.hh>

#include <string>
#include <memory>

#ifdef WIN32
# include <Windows.h>
#else
# include <dlfcn.h>
#endif

BEGIN_NAMESPACE_GMCORE;

/**
   Loads a shared library and loads its nodes into the object factory.
*/
class ImportLibrary
  : public Object {

public:

  ~ImportLibrary();

  /**
     Check if the library has been successfully loaded.
  */
  bool isLoaded() { return library_loaded; }

  /**
     Called to initialize the Object. This should be called once only!
  */
  void initialize();

  /**
     Set name or path of library file.

     \b XML-attribute: \c library
  */
  void setLibrary(std::string file);

  GM_OFI_DECLARE;

private:

  bool library_loaded;
  std::string library;

#ifdef WIN32
  HMODULE handle = 0;
#else
  void *handle = nullptr;
#endif
};

END_NAMESPACE_GMCORE;

#endif
