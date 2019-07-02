
#include <gmTypes/eigen.hh>

#ifdef gramods_ENABLE_Eigen3

#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GRAMODS;

std::istream& operator>> (std::istream &in, Eigen::Vector3f &v) {
  in >> v[0] >> v[1] >> v[2];
  return in;
}

std::istream& operator>> (std::istream &in, Eigen::Quaternionf &q) {

  std::string key;
  in >> key;
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);

  if (key == "quaternion") {

    double w, x, y, z;
    in >> w >> x >> y >> z;

    if (in) {

      q = Eigen::Quaternionf(w, x, y, z);

      if (fabs(1 - q.norm()) > 1e-5)
        GM_WRN("operator>>(std::istream, Eigen::Quaternionf)",
               "Parsed quaternion is not unit (pure rotation).");

    } else {
      q = Eigen::Quaternionf::Identity();
      GM_WRN("operator>>(std::istream, Eigen::Quaternionf)",
             "Cannot parse istream into quaternion.");
    }

  } else if (key == "ypr") {

    double yaw, pitch, roll;
    in >> yaw >> pitch >> roll;

    if (in) {
      q = Eigen::Quaternionf(Eigen::AngleAxisf(-yaw, Eigen::Vector3f::UnitY()) *
                             Eigen::AngleAxisf(pitch, Eigen::Vector3f::UnitX()) *
                             Eigen::AngleAxisf(-roll, Eigen::Vector3f::UnitZ()));
    } else {
      q = Eigen::Quaternionf::Identity();
      GM_WRN("operator>>(std::istream, Eigen::Quaternionf)",
             "Cannot parse istream into Euler angles (ypr).");
    }

  } else if (key == "axisangle") {

    Eigen::Vector3f v;
    in >> v[0] >> v[1] >> v[2];

    float angle;
    in >> angle;

    if (in) {
      auto aa = Eigen::AngleAxisf(angle, v.normalized());
      q = Eigen::Quaternionf(aa);
    } else {
      q = Eigen::Quaternionf::Identity();
      GM_WRN("operator>>(std::istream, Eigen::Quaternionf)",
             "Cannot parse istream into axis angle.");
    }

  } else if (key == "angleaxis") {

    float angle;
    in >> angle;

    Eigen::Vector3f v;
    in >> v[0] >> v[1] >> v[2];

    if (in) {
      auto aa = Eigen::AngleAxisf(angle, v.normalized());
      q = Eigen::Quaternionf(aa);
    } else {
      q = Eigen::Quaternionf::Identity();
      GM_WRN("operator>>(std::istream, Eigen::Quaternionf)",
             "Cannot parse istream into angle axis.");
    }

  } else {

    double w, x, y, z;

    std::stringstream ss(key);
    ss >> w;

    if (!ss) {
      q = Eigen::Quaternionf::Identity();
      GM_WRN("operator>>(std::istream, Eigen::Quaternionf)",
             "Non-parsable (\"" << key << "\") is not recognized as key.");
      return in;
    }

    in >> x >> y >> z;

    if (in) {

      q = Eigen::Quaternionf(w, x, y, z);

      if (fabs(1 - q.norm()) > 1e-5)
        GM_WRN("operator>>(std::istream, Eigen::Quaternionf)",
               "Parsed quaternion is not unit (pure rotation).");

    } else {
      q = Eigen::Quaternionf::Identity();
      GM_WRN("operator>>(std::istream, Eigen::Quaternionf)",
             "Cannot parse istream into quaternion.");
    }
  }

  return in;
}

END_NAMESPACE_GRAMODS;

#endif
