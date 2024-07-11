
#ifndef GRAMODS_GRAPHICS_VIEWPOINT
#define GRAMODS_GRAPHICS_VIEWPOINT

#include <gmGraphics/config.hh>
#include <gmGraphics/Eye.hh>

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_eigen.hh>

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>

#include <optional>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The viewpoint represents a point in space from which the view is to
   be rendered, and an orientation. The orientation may be used for
   view orientation or for eye separation. The view may use this
   Viewpoint to create Camera fitting for rendering.

   Sub classes may implement dynamically updated Viewpoints, based on
   animation or tracking data.
*/
class Viewpoint
  : public gmCore::Object {

public:

  /**
     Returns the position of the viewpoint.

     \param[in] eye The eye for which to fetch position. Default is
     Eye::MONO.
  */
  virtual Eigen::Vector3f getPosition(Eye eye = Eye::MONO);

  /**
     Returns the orientation of the viewpoint. Observe that a view may
     ignore the orientation value if this does not make any difference
     for its appearance.

     \param[in] eye The eye for which to fetch orientation. Default is
     Eye::MONO.
  */
  virtual Eigen::Quaternionf getOrientation(Eye eye = Eye::MONO);

  /**
     Explicitly sets the position of the Viewpoint. This may be
     ignored or instantaneously overwritten by dynamic updates of the
     position value.

     \gmXmlTag{gmGraphics,Viewpoint,position}

     \sa gramods::operator>>(std::istream &, Eigen::Vector3f &)
  */
  virtual void setPosition(Eigen::Vector3f p);

  /**
     Sets the distance between the eyes for stereoscopic (or
     multiscopic) rendering, in internal units, typically
     meters. Default is 0.06 m.

     \gmXmlTag{gmGraphics,Viewpoint,eyeSeparation}
  */
  void setEyeSeparation(float e) {
    eye_separation = e;
  }

  /**
     Explicitly sets the orientation of the Viewpoint.

     This may be ignored or instantaneously overwritten by dynamic
     updates of the position value.

     \gmXmlTag{gmGraphics,Viewpoint,orientation}

     \sa gramods::operator>>(std::istream &, Eigen::Quaternionf &)
  */
  virtual void setOrientation(Eigen::Quaternionf q);

  /**
     Set the up direction to be used in a later call to
     setLookAt. This does nothing if setLookAt is not used.

     \gmXmlTag{gmGraphics,Viewpoint,upDirection}

     \sa gramods::operator>>(std::istream &, Eigen::Vector3f &)
  */
  virtual void setUpDirection(Eigen::Vector3f up);

  /**
     Sets a point that the viewpoint should be rotated towards.

     Observe that this affects only the viewpoint orientation -
     camera orientation will not automatically toe-in to this point.

     \gmXmlTag{gmGraphics,Viewpoint,lookAt}

     \sa gramods::operator>>(std::istream &, Eigen::Vector3f &)
     \sa unsetLookat()
  */
  virtual void setLookAt(Eigen::Vector3f p) { look_at = p; }

  /**
     Unsets the look-at point so that the viewpoint orientation is no
     longer automatically rotated.
  */
  virtual void unsetLookAt() { look_at = std::nullopt; }

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  virtual std::string getDefaultKey() override { return "viewpoint"; }

  GM_OFI_DECLARE;

protected:

  Eigen::Vector3f position = Eigen::Vector3f::Zero();
  Eigen::Quaternionf orientation = Eigen::Quaternionf::Identity();

  Eigen::Vector3f up_direction = Eigen::Vector3f(0, 1, 0);
  std::optional<Eigen::Vector3f> look_at;

  float eye_separation = 0.06f;

};

END_NAMESPACE_GMGRAPHICS;

#endif
