
#ifndef GRAMODS_TRACK_REGISTEREDSINGLEPOSETRACKER
#define GRAMODS_TRACK_REGISTEREDSINGLEPOSETRACKER

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

  /**
     Sets the SinglePoseTracker to register data from.
  */
  void setSinglePoseTracker(std::shared_ptr<SinglePoseTracker> tracker) {
    this->tracker = tracker;
  }

  /**
     Sets the matrix that describes the transform from the tracker
     space to the registered space.
  */
  void setRegistrationMatrix(Eigen::Matrix4f m) {
    registration = m;
  }

  /**
     Sets a position bias to the registration. This is typically not
     needed.
  */
  void setPositionBias(Eigen::Vector3f p) {
    position_bias = p;
  }

  /**
     Sets an orientation bias to the registration. This is typically
     not needed.
  */
  void setOrientationBias(Eigen::Quaternionf q) {
    orientation_bias = q;
  }

  /**
     Replaces the contents of p with pose data.
  */
  bool getPose(PoseSample &p);

  GM_OFI_DECLARE;

private:

  std::shared_ptr<SinglePoseTracker> tracker;

  Eigen::Matrix4f registration = Eigen::Matrix4f::Identity();
  Eigen::Vector3f position_bias = Eigen::Vector3f::Zero();
  Eigen::Quaternionf orientation_bias = Eigen::Quaternionf::Identity();

};

END_NAMESPACE_GMTRACK;

#endif
