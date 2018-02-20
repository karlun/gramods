
#ifndef GRAMODS_CONFIG_IMPORTLIBRARY
#define GRAMODS_CONFIG_IMPORTLIBRARY

#include <gmConfig/config.hh>
#include <gmConfig/OFactoryNode.hh>

BEGIN_NAMESPACE_GMCONFIG

/**
   
 */
class ImportLibrary
  : public OFactoryNode {

public:

  ImportLibrary();
  ~ImportLibrary();

  bool isLoaded() { return library_loaded; }

  void configure(const Configuration &config);

  /** Registers this class with the object factory. */
  static OFactoryInformation<ImportLibrary> node_information;

private:

  bool library_loaded;

};

END_NAMESPACE_GMCONFIG

#endif
