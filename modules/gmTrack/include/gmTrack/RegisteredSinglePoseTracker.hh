
#ifndef GRAMODS_TRACK_REGISTEREDSINGLEPOSETRACKER
#define GRAMODS_TRACK_REGISTEREDSINGLEPOSETRACKER

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_eigen.hh>

#include <gmTrack/SinglePoseTracker.hh>

#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   Provides the pose from another single pose tracker in a registered
   coordinate system.
*/
class RegisteredSinglePoseTracker
  : public SinglePoseTracker {

public:

  RegisteredSinglePoseTracker();
  ~RegisteredSinglePoseTracker();

  /**
     Sets the SinglePoseTracker to register data from.
  */
  void setSinglePoseTracker(std::shared_ptr<SinglePoseTracker> tracker);

  /**
     Sets the matrix that describes the transform from the tracker
     space to the registered space.

     \gmXmlTag{gmTrack,RegisteredSinglePoseTracker,registrationMatrix}

     \sa gramods::operator>>(std::istream &, Eigen::Matrix4f &)
  */
  void setRegistrationMatrix(Eigen::Matrix4f m);

  /**
     Sets a bias to the registration. This is typically not needed.

     \gmXmlTag{gmTrack,RegisteredSinglePoseTracker,biasMatrix}

     \sa gramods::operator>>(std::istream &, Eigen::Matrix4f &)
  */
  void setBiasMatrix(Eigen::Matrix4f m);

  /**
     Sets a position bias to the registration. This is typically not
     needed.

     \gmXmlTag{gmTrack,RegisteredSinglePoseTracker,positionBias}

     \sa gramods::operator>>(std::istream &, Eigen::Vector3f &)
     \sa setBiasMatrix
  */
  void setPositionBias(Eigen::Vector3f p);

  /**
     Sets an orientation bias to the registration. This is typically
     not needed.

     \gmXmlTag{gmTrack,RegisteredSinglePoseTracker,orientationBias}

     \sa gramods::operator>>(std::istream &, Eigen::Quaternionf &)
     \sa setBiasMatrix
  */
  void setOrientationBias(Eigen::Quaternionf q);

  /**
     Replaces the contents of p with pose data.
  */
  bool getPose(PoseSample &p) override;

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
