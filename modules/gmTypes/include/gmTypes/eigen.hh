
#ifndef GRAMODS_TYPES_EIGEN
#define GRAMODS_TYPES_EIGEN

#include <gmTypes/config.hh>

#ifdef gramods_ENABLE_Eigen3

#include <Eigen/Eigen>
#include <iostream>

BEGIN_NAMESPACE_GRAMODS;

/**
   Stream operator reading three values into an Eigen::Vector3f.
*/
inline std::istream& operator>> (std::istream &in, Eigen::Vector3f &v) {
  in >> v[0] >> v[1] >> v[2];
  return in;
}

/**
   Stream operator reading four values (w x y z) into an
   Eigen::Quaternionf.
*/
inline std::istream& operator>> (std::istream &in, Eigen::Quaternionf &q) {
  double w, x, y, z;
  in >> w >> x >> y >> z;
  q = Eigen::Quaternionf(w, x, y, z);
  return in;
}

/**
   Stream operator reading four values (a x y z) into an Eigen::AngleAxisf.
*/
inline std::istream& operator>> (std::istream &in, Eigen::AngleAxisf &aa) {
  float angle;
  in >> angle;
  Eigen::Vector3f v;
  in >> v[0] >> v[1] >> v[2];
  aa = Eigen::AngleAxisf(angle, v.normalized());
  return in;
}

END_NAMESPACE_GRAMODS;

#endif
#endif
