
#ifndef GRAMODS_TYPES_OPERATORS
#define GRAMODS_TYPES_OPERATORS

#include <gmTypes/config.hh>
#include <gmTypes/types.hh>

inline std::istream& operator>> (std::istream &in, gramods::gmTypes::size2 &s) {
  in >> s[0] >> s[1];
  return in;
}

inline std::istream& operator>> (std::istream &in, gramods::gmTypes::size3 &s) {
  in >> s[0] >> s[1] >> s[2];
  return in;
}

inline std::istream& operator>> (std::istream &in, gramods::gmTypes::size4 &s) {
  in >> s[0] >> s[1] >> s[2] >> s[3];
  return in;
}


inline std::istream& operator>> (std::istream &in, gramods::gmTypes::float2 &s) {
  in >> s[0] >> s[1];
  return in;
}

inline std::istream& operator>> (std::istream &in, gramods::gmTypes::float3 &s) {
  in >> s[0] >> s[1] >> s[2];
  return in;
}

inline std::istream& operator>> (std::istream &in, gramods::gmTypes::float4 &s) {
  in >> s[0] >> s[1] >> s[2] >> s[3];
  return in;
}

#endif
