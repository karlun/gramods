

#include <gmTrack/OffsetSinglePoseTracker.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

#include <gmCore/io_eigen.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(OffsetSinglePoseTracker);
GM_OFI_POINTER2(OffsetSinglePoseTracker, singlePoseTracker, SinglePoseTracker, setSinglePoseTracker);
GM_OFI_PARAM2(OffsetSinglePoseTracker, positionOffset, Eigen::Vector3f, setPositionOffset);
GM_OFI_PARAM2(OffsetSinglePoseTracker, orientationOffset, Eigen::Quaternionf, setOrientationOffset);
GM_OFI_PARAM2(OffsetSinglePoseTracker, offsetMatrix, Eigen::Matrix4f, setOffsetMatrix);


bool OffsetSinglePoseTracker::getPose(PoseSample &p) {

  if (!tracker) {
    GM_RUNONCE(GM_WRN("OffsetSinglePoseTracker", "Pose requested but no pose tracker available."));
    return false;
  }

  PoseSample pose;
  if (!tracker->getPose(pose))
    return false;

  p.time = pose.time;

  p.position = pose.position + pose.orientation * position_offset;
  p.orientation = pose.orientation * orientation_offset;

  return true;
}

void OffsetSinglePoseTracker::setOffsetMatrix(Eigen::Matrix4f m) {

  position_offset = m.block<3,1>(0,3);

  Eigen::JacobiSVD<Eigen::MatrixXf> svd(m.block(0,0,3,3),
                                        Eigen::ComputeFullU | Eigen::ComputeFullV);
  auto U = svd.matrixU();
  auto S = svd.singularValues();
  auto V = svd.matrixV();

  Eigen::Matrix3f R = U * V.transpose();
  orientation_offset = Eigen::Quaternionf(R);
}

void OffsetSinglePoseTracker::traverse(Visitor *visitor) {
  if (tracker) tracker->accept(visitor);
}

END_NAMESPACE_GMTRACK;
