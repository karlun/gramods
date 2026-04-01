
#include <gmTrack/BinaryTimeSampleTracker.hh>
#include <gmTrack/TimeSampleTracker.impl.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(BinaryTimeSampleTracker);
GM_OFI_PARAM2(BinaryTimeSampleTracker, key, std::string, addKey);
GM_OFI_PARAM2(BinaryTimeSampleTracker, value, bool, addValue);
GM_OFI_PARAM2(BinaryTimeSampleTracker, time, double, addTime);

template<>
bool BinaryTimeSampleTracker::Impl::interpolate(bool a, bool b, float r) {
  return a;
}

template class TimeSampleTracker<bool>;

END_NAMESPACE_GMTRACK;
