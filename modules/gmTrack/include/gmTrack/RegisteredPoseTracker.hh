
#ifndef GRAMODS_TRACK_REGISTEREDPOSETRACKER
#define GRAMODS_TRACK_REGISTEREDPOSETRACKER

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_eigen.hh>

#include <gmTrack/TrackerBase.hh>

#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   Provides the pose from another single pose tracker in a registered
   coordinate system.
*/
class RegisteredPoseTracker : public PoseTracker {

public:

  RegisteredPoseTracker();
  ~RegisteredPoseTracker();

  /**
     Sets the PoseTracker to register data from.
  */
  void setPoseTracker(std::shared_ptr<PoseTracker> tracker);

  /**
     Adds a specific key to register. All other keys are left
     untouched.
  */
  void addKey(std::string key);

  /**
     Sets the matrix that describes the transform from the tracker
     space to the registered space.

     \gmXmlTag{gmTrack,RegisteredPoseTracker,registrationMatrix}

     \sa gramods::operator>>(std::istream &, Eigen::Matrix4f &)
  */
  void setRegistrationMatrix(Eigen::Matrix4f m);

  /**
     Sets a bias to the registration. This is typically not needed.

     \gmXmlTag{gmTrack,RegisteredPoseTracker,biasMatrix}

     \sa gramods::operator>>(std::istream &, Eigen::Matrix4f &)
  */
  void setBiasMatrix(Eigen::Matrix4f m);

  /**
     Sets a position bias to the registration. This is typically not
     needed.

     \gmXmlTag{gmTrack,RegisteredPoseTracker,positionBias}

     \sa gramods::operator>>(std::istream &, Eigen::Vector3f &)
     \sa setBiasMatrix
  */
  void setPositionBias(Eigen::Vector3f p);

  /**
     Sets an orientation bias to the registration. This is typically
     not needed.

     \gmXmlTag{gmTrack,RegisteredPoseTracker,orientationBias}

     \sa gramods::operator>>(std::istream &, Eigen::Quaternionf &)
     \sa setBiasMatrix
  */
  void setOrientationBias(Eigen::Quaternionf q);

  /**
     @see PoseTracker::get
  */
  std::optional<State> get() override;

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  GM_OFI_DECLARE;

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMTRACK;

#endif
