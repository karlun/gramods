
#ifndef GRAMODS_CORE_IMPORTLIBRARY
#define GRAMODS_CORE_IMPORTLIBRARY

#include <gmCore/config.hh>
#include <gmCore/Object.hh>

#include <string>
#include <memory>

BEGIN_NAMESPACE_GMCORE;

/**
   
 */
class ImportLibrary
  : public Object {

public:

  ImportLibrary();
  ~ImportLibrary();

  bool isLoaded() { return library_loaded; }

  /** @name Parameters
   * Methods used to set and get parameter values.
   */
  ///@{

  void setFile(std::string file);
  std::string getFile();

  void setChild(std::shared_ptr<ImportLibrary> ptr);
  std::shared_ptr<ImportLibrary> getChild();

  ///!@}

private:

  bool library_loaded;
  std::string file;
  std::shared_ptr<ImportLibrary> child;
};

END_NAMESPACE_GMCORE;

#endif
