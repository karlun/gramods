

#include <gmTrack/OffsetSinglePoseTracker.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

#include <gmTypes/eigen.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(OffsetSinglePoseTracker);
GM_OFI_POINTER(OffsetSinglePoseTracker, singlePoseTracker, SinglePoseTracker, OffsetSinglePoseTracker::setSinglePoseTracker);
GM_OFI_PARAM(OffsetSinglePoseTracker, positionOffset, Eigen::Vector3f, OffsetSinglePoseTracker::setPositionOffset);
GM_OFI_PARAM(OffsetSinglePoseTracker, orientationOffset, Eigen::Quaternionf, OffsetSinglePoseTracker::setOrientationOffset);
GM_OFI_PARAM(OffsetSinglePoseTracker, offsetMatrix, Eigen::Matrix4f, OffsetSinglePoseTracker::setOffsetMatrix);


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

END_NAMESPACE_GMTRACK;
