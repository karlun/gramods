
#ifndef GRAMODS_CORE_POSE
#define GRAMODS_CORE_POSE

#include <gmCore/config.hh>

#ifdef gramods_ENABLE_Eigen3

#ifdef gramods_ENABLE_nlohmann_json
#include <nlohmann/json_fwd.hpp>
#endif

#include <Eigen/Eigen>

#include <iostream>

BEGIN_NAMESPACE_GMCORE;

/**
   Type for reading both position and orientation.
*/
struct Pose {
  Eigen::Vector3f position = Eigen::Vector3f::Zero();
  Eigen::Quaternionf orientation = Eigen::Quaternionf::Identity();

  Eigen::Affine3f asMatrix() const {
    return Eigen::Translation3f(position) * orientation;
  }
};

#ifdef gramods_ENABLE_nlohmann_json
void to_json(nlohmann::json &j, const Pose &o);
void from_json(const nlohmann::json &j, Pose &o);
#endif

END_NAMESPACE_GMCORE;

BEGIN_NAMESPACE_GRAMODS;

/**
   Stream operator reading a pose as position and orientation
   separated by a semicolon.

   \sa operator>>(std::istream &, Eigen::Vector3f &)
   \sa operator>>(std::istream &, Eigen::Quaternionf &)
*/
std::istream &operator>>(std::istream &in, gmCore::Pose &p);

/**
   Stream operator reading multiple poses, separated by comma.

   \sa operator>>(std::istream &, gmCore::Pose &)
*/
std::istream &operator>>(std::istream &in, std::vector<gmCore::Pose> &p);

/**
   Stream operator writing/printing a gmCore::Pose as position and
   axisangle orientation, separated by semicolon.

   \sa operator>>(std::istream &, gmCore::Pose &)
*/
std::ostream &operator<<(std::ostream &out, const gmCore::Pose &pose);

END_NAMESPACE_GRAMODS;

#endif
#endif
