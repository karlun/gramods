
#ifndef GRAMODS_TYPES_OPERATORS
#define GRAMODS_TYPES_OPERATORS

#include <gmTypes/config.hh>
#include <gmTypes/types.hh>
#include <iostream>

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

inline std::ostream& operator<< (std::ostream &out, gramods::gmTypes::size2 &s) {
  out << s[0] << " " << s[1];
  return out;
}

inline std::ostream& operator<< (std::ostream &out, gramods::gmTypes::size3 &s) {
  out << s[0] << " " << s[1] << " " << s[2];
  return out;
}

inline std::ostream& operator<< (std::ostream &out, gramods::gmTypes::size4 &s) {
  out << s[0] << " " << s[1] << " " << s[2] << " " << s[3];
  return out;
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

inline std::istream& operator>> (std::istream &in, gramods::gmTypes::float12 &s) {
  in >> s[0] >> s[1] >> s[2] >> s[3] >> s[4] >> s[5] >> s[6] >> s[7] >> s[8] >> s[9] >> s[10] >> s[11];
  return in;
}

inline std::ostream& operator<< (std::ostream &out, gramods::gmTypes::float2 &s) {
  out << s[0] << " " << s[1];
  return out;
}

inline std::ostream& operator<< (std::ostream &out, gramods::gmTypes::float3 &s) {
  out << s[0] << " " << s[1] << " " << s[2];
  return out;
}

inline std::ostream& operator<< (std::ostream &out, gramods::gmTypes::float4 &s) {
  out << s[0] << " " << s[1] << " " << s[2] << " " << s[3];
  return out;
}

#endif
