
#ifndef GM_TRACK_RELATIVETRACKERPOSEREGISTRATIONESTIMATOR
#define GM_TRACK_RELATIVETRACKERPOSEREGISTRATIONESTIMATOR

#include <gmTrack/config.hh>

#include <gmCore/io_eigen.hh>
#include <gmCore/OFactory.hh>
#include <gmCore/Updateable.hh>

#include <gmTrack/MultiPoseTracker.hh>
#include <gmTrack/SinglePoseTracker.hh>
#include <gmTrack/MultiPoseTracker.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   The RelativeTrackerPoseRegistrationEstimator is a utility for
   estimating the relative pose of fixed trackers. These trackers may,
   for example, be attached to a frame that is moving during
   capturing. At least two different trackers are required for this to
   make sense.
*/
class RelativeTrackerPoseRegistrationEstimator : public gmCore::Object {

public:
  RelativeTrackerPoseRegistrationEstimator();
  ~RelativeTrackerPoseRegistrationEstimator();

  /**
     Adds a SinglePoseTracker to register.

     \gmXmlTag{gmTrack,RelativeTrackerPoseRegistrationEstimator,singlePoseTracker}
  */
  void addSinglePoseTracker(std::shared_ptr<SinglePoseTracker> tracker);

  /**
     Adds a MultiPoseTracker to register.

     \gmXmlTag{gmTrack,RelativeTrackerPoseRegistrationEstimator,multiPoseTracker}
  */
  void addMultiPoseTracker(std::shared_ptr<MultiPoseTracker> tracker);

  /**
     Sets the frequency at which samples are collected. Default is
     1. At most one sample per frame will be used regardless of this
     value.

     \gmXmlTag{gmTrack,RelativeTrackerPoseRegistrationEstimator,samplesPerSecond}
  */
  void setSamplesPerSecond(float n);

  /**
     Sets the data offset required to trigger a warning. Default is
     0.01, i.e. one cm.

     \gmXmlTag{gmTrack,RelativeTrackerPoseRegistrationEstimator,warningThreshold}
  */
  void setWarningThreshold(float d);

  /**
     Set the maximum positional distance from the average allowed for
     a sample to be included in the average. Default is -1 meaning
     that all samples are included.
  */
  void setInlierThreshold(float r);

  /**
     Sets the data offset required to trigger a warning. Default is
     π/4 radians.

     \gmXmlTag{gmTrack,RelativeTrackerPoseRegistrationEstimator,orientationWarningThreshold}
  */
  void setOrientationWarningThreshold(float d);

  /**
     Set the maximum orientational distance (in radians) from the
     average allowed for a sample to be included in the
     average. Default is -1 meaning that all samples are included.
  */
  void setOrientationInlierThreshold(float r);

  /**
     Type combining position and orientation.
  */
  struct Pose {
    Eigen::Vector3f position;
    Eigen::Quaternionf orientation;
  };

  /**
     Returns the number of SinglePoseTracker associated with the
     estimator.
  */
  size_t getSinglePoseTrackerCount() const;

  /**
     Returns the pose of the specified SinglePoseTracker.
  */
  Pose getSinglePoseTrackerPose(size_t idx) const;

  /**
     Returns the number of MultiPoseTracker associated with the
     estimator.
  */
  size_t getMultiPoseTrackerCount() const;

  /**
     Returns the poses of the trackeres in the specified
     MultiPoseTracker.
  */
  std::map<int, Pose> getMultiPoseTrackerPose(size_t idx) const;

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

std::ostream &
operator<<(std::ostream &out,
           const RelativeTrackerPoseRegistrationEstimator::Pose &pose) {
  Eigen::AngleAxisf aa(pose.orientation);
  return out << "TPose(P=" << pose.position.transpose() << ", R=" << aa.angle()
             << "/" << aa.axis().transpose() << ")";
}

END_NAMESPACE_GMTRACK;

#endif
