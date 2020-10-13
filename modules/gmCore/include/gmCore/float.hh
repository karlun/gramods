
#ifndef GRAMODS_CORE_FLOAT
#define GRAMODS_CORE_FLOAT

#include <gmCore/config.hh>
#include <array>
#include <iostream>

BEGIN_NAMESPACE_GMCORE;

/// Array of 2 float.
typedef std::array<float, 2> float2;

/// Array of 3 float.
typedef std::array<float, 3> float3;

/// Array of 4 float.
typedef std::array<float, 4> float4;

/// Array of 12 float.
typedef std::array<float, 12> float12;

END_NAMESPACE_GMCORE;

BEGIN_NAMESPACE_GRAMODS;

/// Stream operator reading two values into a float2.
inline std::istream& operator>> (std::istream &in, gramods::gmCore::float2 &s) {
  in >> s[0] >> s[1];
  return in;
}

/// Stream operator reading three values into a float3.
inline std::istream& operator>> (std::istream &in, gramods::gmCore::float3 &s) {
  in >> s[0] >> s[1] >> s[2];
  return in;
}

/// Stream operator reading four values into a float4.
inline std::istream& operator>> (std::istream &in, gramods::gmCore::float4 &s) {
  in >> s[0] >> s[1] >> s[2] >> s[3];
  return in;
}

/// Stream operator reading 12 values into a float12.
inline std::istream& operator>> (std::istream &in, gramods::gmCore::float12 &s) {
  in >> s[0] >> s[1] >> s[2] >> s[3] >> s[4] >> s[5]
     >> s[6] >> s[7] >> s[8] >> s[9] >> s[10] >> s[11];
  return in;
}

/// Stream operator writing two values from a float2.
inline std::ostream& operator<< (std::ostream &out, gramods::gmCore::float2 &s) {
  out << s[0] << " " << s[1];
  return out;
}

/// Stream operator writing three values from a float3.
inline std::ostream& operator<< (std::ostream &out, gramods::gmCore::float3 &s) {
  out << s[0] << " " << s[1] << " " << s[2];
  return out;
}

/// Stream operator writing four values from a float4.
inline std::ostream& operator<< (std::ostream &out, gramods::gmCore::float4 &s) {
  out << s[0] << " " << s[1] << " " << s[2] << " " << s[3];
  return out;
}

/// Stream operator writing 12 values from a float12.
inline std::ostream& operator<< (std::ostream &out, gramods::gmCore::float12 &s) {
  out << s[0] << s[1] << s[2] << s[3] << s[4] << s[5]
      << s[6] << s[7] << s[8] << s[9] << s[10] << s[11];
  return out;
}

END_NAMESPACE_GRAMODS;

#endif
