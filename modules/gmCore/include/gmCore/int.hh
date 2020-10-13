
#ifndef GRAMODS_CORE_INT
#define GRAMODS_CORE_INT

#include <gmCore/config.hh>
#include <array>
#include <iostream>

BEGIN_NAMESPACE_GMCORE;

/// Array of 2 int.
typedef std::array<int, 2> int2;

/// Array of 3 int.
typedef std::array<int, 3> int3;

/// Array of 4 int.
typedef std::array<int, 4> int4;

END_NAMESPACE_GMCORE;

BEGIN_NAMESPACE_GRAMODS;

/// Stream operator reading two values into a int2.
inline std::istream& operator>> (std::istream &in, gramods::gmCore::int2 &s) {
  in >> s[0] >> s[1];
  return in;
}

/// Stream operator reading three values into a int3.
inline std::istream& operator>> (std::istream &in, gramods::gmCore::int3 &s) {
  in >> s[0] >> s[1] >> s[2];
  return in;
}

/// Stream operator reading four values into a int4.
inline std::istream& operator>> (std::istream &in, gramods::gmCore::int4 &s) {
  in >> s[0] >> s[1] >> s[2] >> s[3];
  return in;
}

/// Stream operator writing two values from a int2.
inline std::ostream& operator<< (std::ostream &out, gramods::gmCore::int2 &s) {
  out << s[0] << " " << s[1];
  return out;
}

/// Stream operator writing three values from a int3.
inline std::ostream& operator<< (std::ostream &out, gramods::gmCore::int3 &s) {
  out << s[0] << " " << s[1] << " " << s[2];
  return out;
}

/// Stream operator writing four values from a int4.
inline std::ostream& operator<< (std::ostream &out, gramods::gmCore::int4 &s) {
  out << s[0] << " " << s[1] << " " << s[2] << " " << s[3];
  return out;
}

END_NAMESPACE_GRAMODS;

#endif
