
#include <gmGraphics/PathViewpoint.hh>

#include <gmCore/ExitException.hh>
#include <gmMisc/PolyFit.hh>

#include <chrono>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(PathViewpoint, Viewpoint);
GM_OFI_PARAM2(PathViewpoint, path, std::vector<Pose>, setPath);
GM_OFI_PARAM2(PathViewpoint, loop, bool, setLoop);
GM_OFI_PARAM2(PathViewpoint, exit, bool, setExit);
GM_OFI_PARAM2(PathViewpoint, velocity, float, setVelocity);

struct PathViewpoint::Impl {

  typedef gmCore::Updateable::clock clock;
  typedef std::chrono::duration<double, std::ratio<1>> d_seconds;

  std::vector<Pose> path;
  std::vector<std::unique_ptr<gmMisc::PolyFit>> interpolators;
  bool do_loop = true;
  bool do_exit = false;
  float velocity = 1.0;

  double index = 0.0;
  clock::time_point last_time = clock::now();

  void update(Eigen::Vector3f &position,
              Eigen::Quaternionf &orientation,
              clock::time_point now);
};

PathViewpoint::PathViewpoint()
  : _impl(std::make_unique<Impl>()) {}

void PathViewpoint::update(clock::time_point time, size_t frame) {
  _impl->update(position, orientation, time);
}

namespace {
Eigen::Quaternionf vectorToQuaternion(float x, float y, float z) {
  auto axis = Eigen::Vector3f(x, y, z);
  auto axis_length = axis.norm();
  if (axis_length < std::numeric_limits<float>::epsilon()) {
    return Eigen::Quaternionf::Identity();
  } else {
    return Eigen::Quaternionf(
        Eigen::AngleAxisf(axis_length, axis / axis_length));
  }
}
}



void PathViewpoint::Impl::update(Eigen::Vector3f &position,
                                 Eigen::Quaternionf &orientation,
                                 clock::time_point time) {

  if (path.empty()) return;
  int path_size(path.size());

  if (interpolators.empty()) {
    interpolators.reserve(path.size());
    for (int idx = 0; idx < path_size; ++idx) {
      // t -> (px, py, pz, ra * rx, ra * ry, ra * rz)
      interpolators.emplace_back(std::make_unique<gmMisc::PolyFit>(1, 6, 3));
      for (int s_idx = idx > 0   ? idx - 1
                       : do_loop ? -1
                                 : 0;
           s_idx < (do_loop ? idx + 3 : std::min(path_size, idx + 3));
           ++s_idx) {
        const auto pose = path[(s_idx + path_size) % path_size];
        const auto p = pose.position;
        const Eigen::AngleAxisf r(pose.orientation);
        interpolators.back()->addSample({double(s_idx)},
                                        {p.x(),
                                         p.y(),
                                         p.z(),
                                         r.angle() * r.axis().x(),
                                         r.angle() * r.axis().y(),
                                         r.angle() * r.axis().z()});
      }
    }
  }

  float dt =
      float(std::chrono::duration_cast<d_seconds>(time - last_time).count());

  auto J = interpolators[size_t(index)]->getJacobian({index});
  auto dpdt = Eigen::Vector3f(J(0, 0), J(1, 0), J(2, 0)).norm();

  index += (dt * velocity) / dpdt;

  if (do_exit && index + 1 > path_size) throw gmCore::ExitException(0);
  while (do_loop && index > path_size) index -= path_size;

  auto V = interpolators[size_t(index)]->getValue({index});
  position = Eigen::Vector3f(V[0], V[1], V[2]);
  orientation = vectorToQuaternion(V[3], V[4], V[5]);

  last_time = time;
}

void PathViewpoint::setPath(std::vector<Pose> path) {
  _impl->interpolators.clear();
  _impl->path = path;
}

void PathViewpoint::addNode(const Pose &node) {
  _impl->interpolators.clear();
  _impl->path.push_back(node);
}

void PathViewpoint::setLoop(bool on) { _impl->do_loop = on; }
void PathViewpoint::setExit(bool on) { _impl->do_exit = on; }
void PathViewpoint::setVelocity(float v) { _impl->velocity = v; }

END_NAMESPACE_GMGRAPHICS;
