
#include <gmConfig/ImportLibrary.hh>

USING_NAMESPACE_GMCONFIG;

ImportLibrary::ImportLibrary()
  : library_loaded(false) {}

ImportLibrary::~ImportLibrary(){}

ImportLibrary::OFactoryInformation<ImportLibrary> ImportLibrary::node_information("ImportLibrary");

/** Configures this class. */
void ImportLibrary::configure(const Configuration &config){

  std::string file;
  if (!config.getParam("file", file))
    return;

  library_loaded = true;
}
