
#include <gmCore/io_eigen.hh>

#include <gmTrack/PoseTimeSampleTracker.hh>
#include <gmTrack/TimeSampleTracker.impl.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(PoseTimeSampleTracker);
GM_OFI_PARAM2(PoseTimeSampleTracker, key, std::string, addKey);
GM_OFI_PARAM2(PoseTimeSampleTracker, value, gmCore::Pose, addValue);
GM_OFI_PARAM2(PoseTimeSampleTracker, time, double, addTime);

template<>
gmCore::Pose TimeSampleTracker<gmCore::Pose>::Impl::interpolate(gmCore::Pose a,
                                                                gmCore::Pose b,
                                                                float r){
  return {.position = (1 - r) * a.position + r * b.position,
          .orientation = a.orientation.slerp(r, b.orientation)};
}

template class TimeSampleTracker<gmCore::Pose>;

END_NAMESPACE_GMTRACK;
