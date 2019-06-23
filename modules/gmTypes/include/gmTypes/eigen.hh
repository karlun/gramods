
#ifndef GRAMODS_TYPES_EIGEN
#define GRAMODS_TYPES_EIGEN

#include <gmTypes/config.hh>

#ifdef gramods_ENABLE_Eigen3

#include <Eigen/Eigen>

#include <iostream>

BEGIN_NAMESPACE_GRAMODS;

/**
   Stream operator reading three values into an Eigen::Vector3f. This
   is typically used to read XML position and vector attributes. This
   will read three values (x y z) from the stream.
*/
std::istream& operator>> (std::istream &in, Eigen::Vector3f &v);

/**
   Stream operator reading into an Eigen::Quaternionf. This is
   typically used to read XML orientation and rotation
   attributes. Default behavior is to read four values (w x y z), but
   this behavior can be modified by prefixing with a keyword:

   - quaternion w x y z, e.g. "quaternion 1 0 0 0": Enforce the
     default behaviour.

   - ypr ay ap ar, e.g. "ypr 0.1 0 0": Using Euler angles for
     yaw-pitch-roll rotation, i.e. around axis Y, X and Z counter
     clockwise (right-handed), where every angle is expressed in
     radians.

   - axisangle x y z a, e.g. "axisangle 0 1 0 0.1": Using axis for
     rotation and angle of rotation around this axis, where the angle
     is expressed counter clockwise (right-handed) in radians. The
     axis does not have to be normalized.

   - angleaxis a x y z, e.g. "angleaxis 0.1 0 1 0": Using axis for
     rotation and angle of rotation around this axis, where the angle
     is expressed counter clockwise (right-handed) in radians. The
     axis does not have to be normalized.
*/
std::istream& operator>> (std::istream &in, Eigen::Quaternionf &q);

END_NAMESPACE_GRAMODS;

#endif
#endif