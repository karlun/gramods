
#ifndef GRAMODS_CORE_STRING
#define GRAMODS_CORE_STRING

#include <gmCore/config.hh>

#include <array>
#include <iostream>

BEGIN_NAMESPACE_GMCORE;

/// Array of 2 string.
typedef std::array<std::string, 2> string2;

END_NAMESPACE_GMCORE;

BEGIN_NAMESPACE_GRAMODS;

/// Stream operator reading two values into a string2.
std::istream& operator>> (std::istream &in, gramods::gmCore::string2 &s);

/// Stream operator writing two values from a string2.
std::ostream& operator<< (std::ostream &out, gramods::gmCore::string2 &s);

END_NAMESPACE_GRAMODS;

#endif
