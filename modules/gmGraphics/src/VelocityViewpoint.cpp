
#include <gmGraphics/VelocityViewpoint.hh>

#include <chrono>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(VelocityViewpoint, Viewpoint);
GM_OFI_PARAM(VelocityViewpoint, velocity, gmTypes::float3, VelocityViewpoint::setVelocity);
GM_OFI_PARAM(VelocityViewpoint, quaternionVelocity, gmTypes::float4, VelocityViewpoint::setQuaternionVelocity);
GM_OFI_PARAM(VelocityViewpoint, axisAngleVelocity, gmTypes::float4, VelocityViewpoint::setAxisAngleVelocity);

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

  auto dt = std::chrono::duration_cast<d_seconds>(t - last_time);

  position += dt.count() * linear_velocity;
  orientation *= Eigen::Quaternionf::Identity().slerp(dt.count(), angular_velocity);

  last_time = t;
}

void VelocityViewpoint::setVelocity(gmTypes::float3 vel) {
  _impl->linear_velocity = Eigen::Vector3f(vel[0], vel[1], vel[2]);
}

void VelocityViewpoint::setQuaternionVelocity(gmTypes::float4 rot) {
  _impl->angular_velocity = Eigen::Quaternionf(rot[0], rot[1], rot[2], rot[3]);
}

void VelocityViewpoint::setAxisAngleVelocity(gmTypes::float4 rot) {
  _impl->angular_velocity = Eigen::Quaternionf::AngleAxisType
    (rot[3], Eigen::Vector3f(rot[0], rot[1], rot[2]).normalized());
}

END_NAMESPACE_GMGRAPHICS;
