
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
     The signature of the rendering function that provides the actual
     graphics. See gmGraphics::Dispatcher for more information.
  */
  typedef std::function<void(Camera)> RenderFunction;

  /**
     Computes and returns a projection matrix for the current camera
     and the provided near and far distances.
  */
  Eigen::Matrix4f getProjectionMatrix(float near, float far);

  /**
     Computes and returns a view matrix for the current camera.
  */
  Eigen::Matrix4f getViewMatrix();

private:

  /**
     Frustum planes at distance of 1.0 (meters, typically).
  */
  float left, right, top, bottom;

  /**
     The position of the camera.
  */
  Eigen::Vector3f position;

  /**
     The orientation of the camera.
  */
  Eigen::Quaternionf orientation;

  /**
     The viewport that the camera is supposed to render to. This may
     be of size 0, 2 or 4. A vector of size 0 represents unspecified
     size and position (viewport), size 2 represents viewport size
     (width and height) but unspecified position and size 4 provides a
     full viewport representation (x, y, width, height).
   */
  std::vector<std::size_t> viewport;

};

END_NAMESPACE_GMGRAPHICS;

#endif
