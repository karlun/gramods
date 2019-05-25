
#ifndef GRAMODS_GRAPHICS_CAMERA
#define GRAMODS_GRAPHICS_CAMERA

#include <gmGraphics/config.hh>

#include <gmCore/Console.hh>

#include <Eigen/Eigen>
#include <functional>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The base of graphics Camera implementations.
 */
class Camera {

public:

  /**
     Computes and returns a projection matrix for the current camera
     and the provided near and far distances.
  */
  Eigen::Matrix4f getProjectionMatrix(float near, float far);

  /**
     Computes and returns a view matrix for the current camera.
  */
  Eigen::Affine3f getViewMatrix();

  /**
     Get the position of the camera.
  */
  Eigen::Vector3f getPosition();

  /**
     Get the orientation of the camera.
  */
  Eigen::Quaternionf getOrientation();

  /**
     Explicitly sets the frustum planes at a distance of 1. Near and
     far planes are defined by the renderer that requests a projection
     matrix for the camera.
  */
  void setPlanes(float l, float r, float b, float t) {
    left = l; right = r; top = t; bottom = b;
  }

  /**
     Gets the frustum planes at a distance of 1.
  */
  void getPlanes(float &l, float &r, float &b, float &t) {
    l = left; r = right; t = top; b = bottom;
  }

  /**
     Sets the frustum planes of a symmetric frustum for the camera
     based on horizontal and vertical field-of-view, expressed in
     radians.
  */
  void setFieldOfView(float fov_h, float fov_v);

  /**
     Sets the frustum planes of an asymmetric frustum for the camera
     based on left, right, bottom and top field-of-view, expressed in
     radians. The left field-of-view increase left-wise while the
     right field-of-view increate right-wise.
  */
  void setAngles(float l, float r, float b, float t);

  /**
     Sets the pose of the camera.
  */
  void setPose(Eigen::Vector3f p, Eigen::Quaternionf r) {
    position = p; orientation = r;
  }

private:

  /**
     Frustum planes at distance of 1.0 (meters, typically).
  */
  float left = -1.f, right = 1.f, bottom = -1.f, top = 1.f;

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
