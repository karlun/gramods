
#ifndef GRAMODS_CORE_ANGLE
#define GRAMODS_CORE_ANGLE

#include <gmCore/config.hh>
#include <gmCore/MathConstants.hh>

#include <array>
#include <iostream>

BEGIN_NAMESPACE_GMCORE;

/**
   A simple placeholder for angle values expressed in radians.

   For convenience there are means available for converting to and
   from degrees, and there is a stream operator defined that allows
   angles to be expressed in degrees by prepending a 'd', as in d180.
*/
struct angle {

public:

  angle() {}

  angle(float v)
    : value(v) {}

  /// Set the angle, in degrees.
  void deg(float v) { value = v * from_degrees; }

  /// The angle, in degrees.
  float deg() { return value * to_degrees; }

  /// Set the angle, in radians.
  void rad(float v) { value = v; }

  /// The angle, in radians.
  float rad() { return value; }

  /// Set the value of the angle, in radians.
  void operator =(float v) { value = v; }

  /// Cast the angle type to float, still in radians.
  operator float() { return value; }

  /// The angle in radians.
  float value;

  /// Multiplier to convert from degrees to radians.
  static constexpr float from_degrees = (float)(GM_PI / 180.0);

  /// Multiplier to convert from radians to degrees.
  static constexpr float to_degrees = (float)(180.f / GM_PI);

};

/// Array of 2 angle.
typedef std::array<angle, 2> angle2;

/// Array of 3 angle.
typedef std::array<angle, 3> angle3;

/// Array of 4 angle.
typedef std::array<angle, 4> angle4;

END_NAMESPACE_GMCORE;

BEGIN_NAMESPACE_GRAMODS;

/**
   Stream operator reading into a angle. This is typically used to
   read XML angle attributes. Default behavior is to read a single
   value in radians, but the value is interpreted as degrees if it is
   prepended by a 'd' (as in 'd180'). The value can also be prepended
   by an 'r' (as in r3.14) to suppress the warnings that degrees
   probably was ment for large values.
*/
std::istream& operator>> (std::istream &in, gmCore::angle &v);

/// Stream operator reading two values into a angle2.
inline std::istream& operator>> (std::istream &in, gramods::gmCore::angle2 &s) {
  in >> s[0] >> s[1];
  return in;
}

/// Stream operator reading three values into a angle3.
inline std::istream& operator>> (std::istream &in, gramods::gmCore::angle3 &s) {
  in >> s[0] >> s[1] >> s[2];
  return in;
}

/// Stream operator reading four values into a angle4.
inline std::istream& operator>> (std::istream &in, gramods::gmCore::angle4 &s) {
  in >> s[0] >> s[1] >> s[2] >> s[3];
  return in;
}

/// Stream operator writing two values from a angle2.
inline std::ostream& operator<< (std::ostream &out, gramods::gmCore::angle2 &s) {
  out << s[0] << " " << s[1];
  return out;
}

/// Stream operator writing three values from a angle3.
inline std::ostream& operator<< (std::ostream &out, gramods::gmCore::angle3 &s) {
  out << s[0] << " " << s[1] << " " << s[2];
  return out;
}

/// Stream operator writing four values from a angle4.
inline std::ostream& operator<< (std::ostream &out, gramods::gmCore::angle4 &s) {
  out << s[0] << " " << s[1] << " " << s[2] << " " << s[3];
  return out;
}

END_NAMESPACE_GRAMODS;

#endif
