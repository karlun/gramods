
#ifndef GRAMODS_GRAPHICS_CAMERA
#define GRAMODS_GRAPHICS_CAMERA

#include <gmGraphics/config.hh>

#include <gmGraphics/Eye.hh>
#include <gmCore/Console.hh>

#include <Eigen/Eigen>
#include <functional>
#include <optional>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The base of graphics Camera implementations.
 */
class Camera {

public:
  /**
     Creates a camera for rendering the specified frame number.
  */
  Camera(size_t frame_number) : frame_number(frame_number) {}

  Camera(const Camera &other)
    : frame_number(other.frame_number),
      left(other.left),
      right(other.right),
      bottom(other.bottom),
      top(other.top),
      near(other.near),
      far(other.far),
      position(other.position),
      orientation(other.orientation),
      eye(other.eye) {}

  Camera &operator=(const Camera &other) {
    if (frame_number != other.frame_number)
      throw gmCore::InvalidArgument(
          GM_STR("Cannot assign with camera from different frame ("
                 << frame_number << " != " << other.frame_number << ")"));

    left = other.left;
    right = other.right;
    bottom = other.bottom;
    top = other.top;
    near = other.near;
    far = other.far;
    position = other.position;
    orientation = other.orientation;
    eye = other.eye;

    return *this;
  }

  /**
     Creates a camera copying frame number from another object.
  */
  template<class T>
  Camera(const T &object) : frame_number(object.frame_number) {}

  /**
     Computes and returns a projection matrix for the current camera
     and near and far distances.
  */
  Eigen::Matrix4f getProjectionMatrix() const;

  /**
     Computes and returns a view matrix for the current camera.
  */
  Eigen::Affine3f getViewMatrix() const;

  /**
     Get the position of the camera.
  */
  Eigen::Vector3f getPosition() const { return position; }

  /**
     Set the position of the camera.
  */
  void setPosition(Eigen::Vector3f p) { position = p; }

  /**
     Get the orientation of the camera.
  */
  Eigen::Quaternionf getOrientation() const { return orientation; }

  /**
     Set the orientation of the camera.
  */
  void setOrientation(Eigen::Quaternionf q) { orientation = q; }

  /**
     Explicitly sets the frustum clip planes at a distance of 1. Near
     and far planes are defined by the renderer that requests a
     projection matrix for the camera.
  */
  void setClipPlanes(float l, float r, float b, float t) {
    left = l;
    right = r;
    top = t;
    bottom = b;
  }

  /**
     Gets the frustum clip planes at a distance of 1.
  */
  void getClipPlanes(float &l, float &r, float &b, float &t) const {
    l = left;
    r = right;
    t = top;
    b = bottom;
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
     right field-of-view increase right-wise.
  */
  void setClipAngles(float l, float r, float b, float t);

  /**
     Sets the pose of the camera.
  */
  void setPose(Eigen::Vector3f p, Eigen::Quaternionf r) {
    position = p;
    orientation = r;
  }

  /**
     Set the position, orientation and frustum of this camera based on
     a eye position and a set of points that should be made visible
     from this point.
  */
  bool setLookAtPoints(Eigen::Vector3f p,
                       const std::vector<Eigen::Vector3f> &pts,
                       bool symmetric = true,
                       Eigen::Vector3f up = Eigen::Vector3f::Zero());

  /**
     Sets the near and far planes.
  */
  void setNearFar(float near, float far) {
    this->near = near;
    this->far = far;
  }

  /**
     Sets which eye the camera is supposed to render.
  */
  void setEye(Eye e) { eye = e; }

  /**
     Sets which eye the camera is supposed to render. Use this for
     example to select between left or right eye textures.
  */
  Eye getEye() const { return eye; }

  /**
     The frame currently being rendered.
  */
  const size_t frame_number;

private:
  /**
     Frustum planes at distance of 1.0 (meters, typically).
  */
  float left = -1.f, right = 1.f, bottom = -1.f, top = 1.f;

  /**
     Near and far planes.
  */
  std::optional<float> near, far;

  /**
     The position of the camera.
  */
  Eigen::Vector3f position = Eigen::Vector3f::Zero();

  /**
     The orientation of the camera.
  */
  Eigen::Quaternionf orientation = Eigen::Quaternionf::Identity();

  /**
     The current eye being for the camera to render.
  */
  Eye eye = Eye::MONO;
};

END_NAMESPACE_GMGRAPHICS;

#endif
