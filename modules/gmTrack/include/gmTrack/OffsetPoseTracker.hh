
#ifndef GRAMODS_TRACK_OFFSETPOSETRACKER
#define GRAMODS_TRACK_OFFSETPOSETRACKER

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_eigen.hh>

#include <gmTrack/TrackerBase.hh>

#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   Provides the pose as an offset from another pose tracker.
*/
class OffsetPoseTracker : public PoseTracker {

public:

  OffsetPoseTracker();
  virtual ~OffsetPoseTracker();

  /**
     Sets the PoseTracker to register data from.
  */
  void setPoseTracker(std::shared_ptr<PoseTracker> tracker);

  /**
     Adds a specific key to offset. All other keys are left
     untouched.
  */
  void addKey(std::string key);

  /**
     Sets the position offset, in local tracker coordinates, to add to
     the pose data.

     \gmXmlTag{gmTrack,OffsetPoseTracker,positionOffset}

     \sa gramods::operator>>(std::istream &, Eigen::Vector3f &)
  */
  void setPositionOffset(Eigen::Vector3f p);

  /**
     Sets the orientation offset, in local tracker coordinates, to add
     to the pose data.

     \gmXmlTag{gmTrack,OffsetPoseTracker,orientationOffset}

     \sa gramods::operator>>(std::istream &, Eigen::Quaternionf &)
  */
  void setOrientationOffset(Eigen::Quaternionf q);

  /**
     Sets the orientation and position offset by extracting these data
     from an offset matrix.

     \gmXmlTag{gmTrack,OffsetPoseTracker,offsetMatrix}

     \sa gramods::operator>>(std::istream &, Eigen::Matrix4f &)
  */
  void setOffsetMatrix(Eigen::Matrix4f m);

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
