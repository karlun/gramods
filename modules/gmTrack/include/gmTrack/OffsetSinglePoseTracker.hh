
#ifndef GRAMODS_TRACK_OFFSETSINGLEPOSETRACKER
#define GRAMODS_TRACK_OFFSETSINGLEPOSETRACKER

#include <gmTrack/SinglePoseTracker.hh>

#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   Provides the pose as an offset from another single pose tracker.
*/
class OffsetSinglePoseTracker
  : public SinglePoseTracker {

public:

  /**
     Sets the SinglePoseTracker to register data from.
  */
  void setSinglePoseTracker(std::shared_ptr<SinglePoseTracker> tracker) {
    this->tracker = tracker;
  }

  /**
     Sets the position offset, in local tracker coordinates, to add to
     the pose data.
  */
  void setPositionOffset(Eigen::Vector3f p) {
    position_offset = p;
  }

  /**
     Sets the orientation offset, in local tracker coordinates, to add to the pose
     data.
  */
  void setOrientationOffset(Eigen::Quaternionf q) {
    orientation_offset = q;
  }

  /**
     Replaces the contents of p with pose data.
  */
  bool getPose(PoseSample &p);

  GM_OFI_DECLARE;

private:

  std::shared_ptr<SinglePoseTracker> tracker;

  Eigen::Vector3f position_offset = Eigen::Vector3f::Zero();
  Eigen::Quaternionf orientation_offset = Eigen::Quaternionf::Identity();

};

END_NAMESPACE_GMTRACK;

#endif
