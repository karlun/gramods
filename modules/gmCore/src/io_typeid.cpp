
#include <gmCore/io_typeid.hh>

#ifdef __GNUG__
#include <cxxabi.h>
#endif

BEGIN_NAMESPACE_GRAMODS;

std::string demangle(const std::type_info &id) {
  std::string type_name = id.name();

#ifdef __GNUG__
  int status;
  char *dname = abi::__cxa_demangle(type_name.c_str(), NULL, NULL, &status);
  if (!status) type_name = dname;
  std::free(dname);
#endif

  return type_name;
}

std::ostream &operator<<(std::ostream &out, const std::type_info &id) {
  return out << demangle(id);
}

END_NAMESPACE_GRAMODS;
