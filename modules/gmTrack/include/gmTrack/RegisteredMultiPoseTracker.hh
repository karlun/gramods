
#ifndef GRAMODS_TRACK_REGISTEREDMULTIPOSETRACKER
#define GRAMODS_TRACK_REGISTEREDMULTIPOSETRACKER

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

  /**
     Sets the MultiPoseTracker to register data from.
  */
  void setMultiPoseTracker(std::shared_ptr<MultiPoseTracker> tracker) {
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
     Replaces the contents of p with pose data. Returns true if data
     could be read, false otherwise. Use sample time to check if data
     are fresh.
  */
  bool getPose(std::map<int, PoseSample> &p);

  GM_OFI_DECLARE;

private:

  std::shared_ptr<MultiPoseTracker> tracker;

  Eigen::Matrix4f registration = Eigen::Matrix4f::Identity();
  Eigen::Vector3f position_bias = Eigen::Vector3f::Zero();
  Eigen::Quaternionf orientation_bias = Eigen::Quaternionf::Identity();

};

END_NAMESPACE_GMTRACK;

#endif
