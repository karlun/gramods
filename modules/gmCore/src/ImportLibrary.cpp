
#include <gmCore/ImportLibrary.hh>

#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMCORE;

namespace ImportLibraryInternals {
  OFactory::OFactoryInformation<ImportLibrary> OFI("ImportLibrary");
  OFI_PARAM(OFI, ImportLibrary, file, std::string, ImportLibrary::setFile);
  OFI_POINTER(OFI, ImportLibrary, child, ImportLibrary, ImportLibrary::setChild);
}

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

END_NAMESPACE_GMCORE;
