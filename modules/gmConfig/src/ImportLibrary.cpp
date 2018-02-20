
#include <gmConfig/ImportLibrary.hh>

#include <gmConfig/OFactory.hh>

BEGIN_NAMESPACE_GMCONFIG
namespace ImportLibraryInternals {
  OFactory::OFactoryInformation<ImportLibrary> OFI("ImportLibrary");
  OFI_SETTER(OFI, file, std::string, ImportLibrary::setFile);
}
END_NAMESPACE_GMCONFIG

USING_NAMESPACE_GMCONFIG;

ImportLibrary::ImportLibrary()
  : library_loaded(false) {}

ImportLibrary::~ImportLibrary(){}

void ImportLibrary::setFile(std::string file) {
  this->file = file;
  library_loaded = true;
}

std::string ImportLibrary::getFile() {
  return file;
}
