
#ifndef GRAMODS_CORE_SIZE
#define GRAMODS_CORE_SIZE

#include <gmCore/config.hh>
#include <array>
#include <iostream>

BEGIN_NAMESPACE_GMCORE;

/// Array of 2 size_t.
typedef std::array<size_t, 2> size2;

/// Array of 3 size_t.
typedef std::array<size_t, 3> size3;

/// Array of 4 size_t.
typedef std::array<size_t, 4> size4;

END_NAMESPACE_GMCORE;

BEGIN_NAMESPACE_GRAMODS;

/// Stream operator reading two values into a size2.
inline std::istream& operator>> (std::istream &in, gramods::gmCore::size2 &s) {
  in >> s[0] >> s[1];
  return in;
}

/// Stream operator reading three values into a size3.
inline std::istream& operator>> (std::istream &in, gramods::gmCore::size3 &s) {
  in >> s[0] >> s[1] >> s[2];
  return in;
}

/// Stream operator reading four values into a size4.
inline std::istream& operator>> (std::istream &in, gramods::gmCore::size4 &s) {
  in >> s[0] >> s[1] >> s[2] >> s[3];
  return in;
}

/// Stream operator writing two values from a size2.
inline std::ostream& operator<< (std::ostream &out, gramods::gmCore::size2 &s) {
  out << s[0] << " " << s[1];
  return out;
}

/// Stream operator writing three values from a size3.
inline std::ostream& operator<< (std::ostream &out, gramods::gmCore::size3 &s) {
  out << s[0] << " " << s[1] << " " << s[2];
  return out;
}

/// Stream operator writing four values from a size4.
inline std::ostream& operator<< (std::ostream &out, gramods::gmCore::size4 &s) {
  out << s[0] << " " << s[1] << " " << s[2] << " " << s[3];
  return out;
}

END_NAMESPACE_GRAMODS;

#endif
