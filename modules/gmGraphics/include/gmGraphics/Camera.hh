
#ifndef GRAMODS_GRAPHICS_CAMERA
#define GRAMODS_GRAPHICS_CAMERA

#include <gmGraphics/config.hh>

#include <gmCore/Object.hh>

#include <Eigen/Eigen>
#include <functional>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The base of graphics Camera implementations.
 */
class Camera
  : public gmCore::Object {

public:

  /**
     Computes and returns a projection matrix for the current camera
     and the provided near and far distances.
  */
  Eigen::Matrix4f getProjectionMatrix(float near, float far);

  /**
     Computes and returns a view matrix for the current camera.
  */
  Eigen::Matrix4f getViewMatrix();

  void setPlanes(float l, float r, float t, float b) {
    left = l; right = r; top = t; bottom = b;
  }

  void setPose(Eigen::Vector3f p, Eigen::Quaternionf r) {
    position = p; orientation = r;
  }

private:

  /**
     Frustum planes at distance of 1.0 (meters, typically).
  */
  float left = 1.f, right = 1.f, top = 1.f, bottom = 1.f;

  /**
     The position of the camera.
  */
  Eigen::Vector3f position;

  /**
     The orientation of the camera.
  */
  Eigen::Quaternionf orientation;
};

END_NAMESPACE_GMGRAPHICS;

#endif
