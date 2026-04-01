
#ifndef GRAMODS_CORE_TYPEID
#define GRAMODS_CORE_TYPEID

#include <gmCore/config.hh>

#include <iostream>

BEGIN_NAMESPACE_GRAMODS;

/**
   Extracts the name of a type_info, demangling when necessary.
*/
std::string demangle(const std::type_info &id);

/**
   Stream operator for writing type_info name to a stream. This will
   demangle when necessary. Usage: `out << typeid(*ptr);`
*/
std::ostream &operator<<(std::ostream &out, const std::type_info &id);

END_NAMESPACE_GRAMODS;

#endif
