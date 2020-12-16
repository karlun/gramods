
#include <gmGraphics/VelocityViewpoint.hh>

#include <chrono>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(VelocityViewpoint, Viewpoint);
GM_OFI_PARAM2(VelocityViewpoint, velocity, Eigen::Vector3f, setVelocity);
GM_OFI_PARAM2(VelocityViewpoint, rotation, Eigen::Quaternionf, setRotation);

struct VelocityViewpoint::Impl {

  typedef gmCore::Updateable::clock clock;
  typedef std::chrono::duration<double, std::ratio<1>> d_seconds;

  Eigen::Vector3f linear_velocity = Eigen::Vector3f::Zero();
  Eigen::Quaternionf angular_velocity = Eigen::Quaternionf::Identity();

  clock::time_point last_time = clock::now();

  void update(Eigen::Vector3f &position,
              Eigen::Quaternionf &orientation,
              clock::time_point now);
  void updatePosition(Eigen::Vector3f &position, d_seconds dt);
  void updateOrientation(Eigen::Quaternionf &orientation, d_seconds dt);
};

VelocityViewpoint::VelocityViewpoint()
  : _impl(std::make_unique<Impl>()) {}

void VelocityViewpoint::update(clock::time_point t) {
  _impl->update(position, orientation, t);
}

void VelocityViewpoint::Impl::update(Eigen::Vector3f &position,
                                     Eigen::Quaternionf &orientation,
                                     clock::time_point t) {

  float dt = float(std::chrono::duration_cast<d_seconds>(t - last_time).count());

  position += dt * linear_velocity;
  orientation *= Eigen::Quaternionf::Identity().slerp(dt, angular_velocity);

  last_time = t;
}

void VelocityViewpoint::setVelocity(Eigen::Vector3f vel) {
  _impl->linear_velocity = vel;
}

void VelocityViewpoint::setRotation(Eigen::Quaternionf q) {
  _impl->angular_velocity = q;
}

END_NAMESPACE_GMGRAPHICS;
