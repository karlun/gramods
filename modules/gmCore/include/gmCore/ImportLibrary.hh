
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

  ImportLibrary();
  ~ImportLibrary();

  /**
     Check if the library has been successfully loaded.
  */
  bool isLoaded();

  /**
     Called to initialize the Object. This should be called once only!
  */
  void initialize();

  /**
     Set name or path of library file.

     \gmXmlTag{gmCore,ImportLibrary,library}
  */
  void setLibrary(std::string file);

  /**
     Set a string to prepend to the library name. Default is "lib" on
     Linux and the empty string on Windows.
  */
  void setPrefix(std::string str);

  /**
     Set a string to append to the library name. Default is ".so" on
     Linux and ".dll" on Windows. Observe that this string includes
     the dot.
  */
  void setSuffix(std::string str);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMCORE;

#endif
