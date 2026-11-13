
#include <gmCore/Pose.hh>

#ifdef gramods_ENABLE_Eigen3

#include <gmCore/io_eigen.hh>

#include <gmCore/Console.hh>

#ifdef gramods_ENABLE_nlohmann_json
#include <nlohmann/json.hpp>
#endif

#include <cctype>

#ifdef gramods_ENABLE_nlohmann_json
BEGIN_NAMESPACE_GMCORE;

void to_json(nlohmann::json &j, const gramods::gmCore::Pose &o) {
  j = nlohmann::json{
      {"position", o.position},
      {"orientation", o.orientation},
  };
}

void from_json(const nlohmann::json &j, gramods::gmCore::Pose &o) {
  o.position = j.at("position").get<Eigen::Vector3f>();
  o.orientation = j.at("orientation").get<Eigen::Quaternionf>();
}

END_NAMESPACE_GMCORE;
#endif

BEGIN_NAMESPACE_GRAMODS;

std::istream &operator>>(std::istream &in, gmCore::Pose &p) {
  {
    Eigen::Vector3f val;
    if (!(in >> val)) return in;
    p.position = val;
  }
  if (!in) return in;

  while (in && in.peek() == ' ') in.get();

  if (in && in.peek() == ';') {
    in.get();
    Eigen::Quaternionf val;
    if (!(in >> val)) return in;
    p.orientation = val;
  } else {
    in.clear();
  }

  return in;
}

std::istream &operator>>(std::istream &in, std::vector<gmCore::Pose> &p) {
  for (std::string line; std::getline(in, line, ',');) {
    std::stringstream str_in(line);
    gmCore::Pose val;
    if (str_in >> val) {
      p.push_back(val);
      continue;
    }

    in.setstate(std::ios_base::failbit);
    return in;
  }

  in.clear();
  return in;
}

std::ostream &operator<<(std::ostream &out, const gmCore::Pose &pose) {
  Eigen::AngleAxisf aa(pose.orientation);
  return out << pose.position.transpose() << "; axisangle "
             << aa.axis().transpose() << " " << aa.angle();
}

END_NAMESPACE_GRAMODS;

#endif
