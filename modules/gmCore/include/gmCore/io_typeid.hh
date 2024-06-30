
#ifndef GRAMODS_CORE_TYPEID
#define GRAMODS_CORE_TYPEID

#include <gmCore/config.hh>

#ifdef __GNUG__
#include <cxxabi.h>
#endif

#include <iostream>

BEGIN_NAMESPACE_GRAMODS;

/**
   Stream operator for writing type_info name to a stream. This will
   demangle when necessary. Usage: `out << typeid(*ptr);`
*/
std::ostream &operator<<(std::ostream &out, const std::type_info &id) {
  std::string type_name = id.name();

#ifdef __GNUG__
  int status;
  char *dname = abi::__cxa_demangle(type_name.c_str(), NULL, NULL, &status);
  if (!status) type_name = dname;
  std::free(dname);
#endif

  return out << type_name;
}

END_NAMESPACE_GRAMODS;

#endif
