
#include <gmConfig/ImportLibrary.hh>

#include <gmConfig/OFactory.hh>

BEGIN_NAMESPACE_GMCONFIG
namespace ImportLibraryInternals {
  OFactory::OFactoryInformation<ImportLibrary> OFI("ImportLibrary");
  OFI_PARAM(OFI, file, std::string, ImportLibrary::setFile);
  OFI_POINTER(OFI, child, ImportLibrary, ImportLibrary::setChild);
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

void ImportLibrary::setChild(std::shared_ptr<ImportLibrary> ptr) {
  child = ptr;
}

std::shared_ptr<ImportLibrary> ImportLibrary::getChild() {
  return child;
}
