
#include <gmTrack/SingleToMultiPoseTracker.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>


BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(SingleToMultiPoseTracker);
GM_OFI_POINTER2(SingleToMultiPoseTracker, singlePoseTracker, SinglePoseTracker, addSinglePoseTracker);


bool SingleToMultiPoseTracker::getPose(std::map<int, PoseSample> &p) {

  if (trackers.empty()) {
    GM_RUNONCE(GM_WRN("SingleToMultiPoseTracker", "Pose requested but no pose tracker available."));
    return false;
  }

  bool got_data = false;
  for (int idx = 0; idx < trackers.size(); ++idx) {

    PoseSample pose;
    if (!trackers[idx]->getPose(pose)) continue;

    got_data = true;
    p[idx] = pose;
  }

  return got_data;
}

void SingleToMultiPoseTracker::traverse(Visitor *visitor) {
  for (auto &t : trackers) t->accept(visitor);
}

END_NAMESPACE_GMTRACK;
