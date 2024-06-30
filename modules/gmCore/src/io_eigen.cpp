
#include <gmCore/io_eigen.hh>

#ifdef gramods_ENABLE_Eigen3

#include <gmCore/io_angle.hh>

#include <gmCore/Console.hh>

#include <cctype>

BEGIN_NAMESPACE_GRAMODS;

std::istream& operator>> (std::istream &in, Eigen::Vector2f &v) {
  in >> v[0] >> v[1];
  return in;
}

std::istream& operator>> (std::istream &in, Eigen::Vector3f &v) {
  in >> v[0] >> v[1] >> v[2];
  return in;
}

std::istream& operator>> (std::istream &in, Eigen::Quaternionf &q) {

  std::string key;
  in >> key;
  std::transform(key.begin(), key.end(), key.begin(),
                 [](unsigned char c){ return std::tolower(c); });

  if (key == "quaternion") {

    float w, x, y, z;
    in >> w >> x >> y >> z;

    if (in) {

      q = Eigen::Quaternionf(w, x, y, z);

      if (fabsf(1 - q.norm()) > 1e-5)
        GM_WRN("operator>>(std::istream, Eigen::Quaternionf)",
               "Parsed quaternion is not unit (pure rotation).");

    } else {
      q = Eigen::Quaternionf::Identity();
      GM_WRN("operator>>(std::istream, Eigen::Quaternionf)",
             "Cannot parse istream into quaternion.");
    }

  } else if (key == "ypr") {

    gmCore::angle yaw, pitch, roll;
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

    gmCore::angle angle;
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

    gmCore::angle angle;
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

    float w, x, y, z;

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

      if (fabsf(1 - q.norm()) > 1e-5)
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

std::istream& operator>> (std::istream &in, Eigen::Matrix3f &m) {

  float a1, a2, a3, a4, a5, a6, a7, a8, a9;

  in >> a1 >> a2 >> a3 >> a4 >> a5 >> a6 >> a7 >> a8 >> a9;
  m << a1, a2, a3, a4, a5, a6, a7, a8, a9;

  return in;
}

std::istream& operator>> (std::istream &in, Eigen::Matrix4f &m) {

  float a01, a02, a03, a04, a05, a06, a07, a08, a09;
  in >> a01 >> a02 >> a03 >> a04 >> a05 >> a06 >> a07 >> a08 >> a09;

  if (!in) {
    GM_DBG3("eigen",
            "Failed to read 9 values from stream into Eigen::Matrix4f");
    return in;
  }

  float a10, a11, a12;
  in >> a10 >> a11 >> a12;

  if (!in) {
    GM_DBG3("eigen",
            "Read 9 values, but not 12, from stream into Eigen::Matrix4f");
    m <<
      a01, a02, a03, 0.f,
      a04, a05, a06, 0.f,
      a07, a08, a09, 0.f,
      0.f, 0.f, 0.f, 1.f;
    in.clear();
    return in;
  }

  float a13, a14, a15, a16;
  in >> a13 >> a14 >> a15 >> a16;

  if (!in) {
    GM_DBG3("eigen",
            "Read 12 values, but not 16, from stream into Eigen::Matrix4f");
    m <<
      a01, a02, a03, a04,
      a05, a06, a07, a08,
      a09, a10, a11, a12,
      0.f, 0.f, 0.f, 1.f;
    in.clear();
    return in;
  }

  GM_DBG3("eigen", "Read 16 values from stream into Eigen::Matrix4f");
  m <<
    a01, a02, a03, a04,
    a05, a06, a07, a08,
    a09, a10, a11, a12,
    a13, a14, a15, a16;

  return in;
}

END_NAMESPACE_GRAMODS;

#endif
