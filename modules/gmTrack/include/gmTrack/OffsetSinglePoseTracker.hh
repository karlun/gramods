
#ifndef GRAMODS_TRACK_OFFSETSINGLEPOSETRACKER
#define GRAMODS_TRACK_OFFSETSINGLEPOSETRACKER

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_eigen.hh>

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

     \gmXmlTag{gmTrack,OffsetSinglePoseTracker,positionOffset}

     \sa gramods::operator>>(std::istream &, Eigen::Vector3f &)
  */
  void setPositionOffset(Eigen::Vector3f p) {
    position_offset = p;
  }

  /**
     Sets the orientation offset, in local tracker coordinates, to add
     to the pose data.

     \gmXmlTag{gmTrack,OffsetSinglePoseTracker,orientationOffset}

     \sa gramods::operator>>(std::istream &, Eigen::Quaternionf &)
  */
  void setOrientationOffset(Eigen::Quaternionf q) {
    orientation_offset = q;
  }

  /**
     Sets the orientation and position offset by extracting these data
     from an offset matrix.

     \gmXmlTag{gmTrack,OffsetSinglePoseTracker,offsetMatrix}

     \sa gramods::operator>>(std::istream &, Eigen::Matrix4f &)
  */
  void setOffsetMatrix(Eigen::Matrix4f m);

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

  std::shared_ptr<SinglePoseTracker> tracker;

  Eigen::Vector3f position_offset = Eigen::Vector3f::Zero();
  Eigen::Quaternionf orientation_offset = Eigen::Quaternionf::Identity();

};

END_NAMESPACE_GMTRACK;

#endif
