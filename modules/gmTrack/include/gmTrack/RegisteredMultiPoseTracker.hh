
#ifndef GRAMODS_TRACK_REGISTEREDMULTIPOSETRACKER
#define GRAMODS_TRACK_REGISTEREDMULTIPOSETRACKER

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_eigen.hh>

#include <gmTrack/MultiPoseTracker.hh>

#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   Provides the pose from another multi pose tracker in a registered
   coordinate system.
*/
class RegisteredMultiPoseTracker
  : public MultiPoseTracker {

public:

  RegisteredMultiPoseTracker();
  ~RegisteredMultiPoseTracker();

  /**
     Sets the MultiPoseTracker to register data from.
  */
  void setMultiPoseTracker(std::shared_ptr<MultiPoseTracker> tracker);

  /**
     Sets the matrix that describes the transform from the tracker
     space to the registered space.

     \gmXmlTag{gmTrack,RegisteredMultiPoseTracker,registrationMatrix}

     \sa gramods::operator>>(std::istream &, Eigen::Matrix4f &)
  */
  void setRegistrationMatrix(Eigen::Matrix4f m);

  /**
     Sets the matrix that describes the transform from the tracker
     space to the registered space.

     \gmXmlTag{gmTrack,RegisteredMultiPoseTracker,registrationMatrix}

     \sa gramods::operator>>(std::istream &, Eigen::Matrix4f &)
  */
  void setBiasMatrix(Eigen::Matrix4f m);

  /**
     Sets a position bias to the registration. This is typically not
     needed.

     \gmXmlTag{gmTrack,RegisteredMultiPoseTracker,positionBias}

     \sa gramods::operator>>(std::istream &, Eigen::Vector3f &)
  */
  void setPositionBias(Eigen::Vector3f p);

  /**
     Sets an orientation bias to the registration. This is typically
     not needed.

     \gmXmlTag{gmTrack,RegisteredMultiPoseTracker,orientationBias}

     \sa gramods::operator>>(std::istream &, Eigen::Quaternionf &)
  */
  void setOrientationBias(Eigen::Quaternionf q);

  /**
     Replaces the contents of p with pose data. Returns true if data
     could be read, false otherwise. Use sample time to check if data
     are fresh.
  */
  bool getPose(std::map<int, PoseSample> &p) override;

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
