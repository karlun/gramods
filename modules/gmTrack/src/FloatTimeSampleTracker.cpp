
#include <gmTrack/FloatTimeSampleTracker.hh>
#include <gmTrack/TimeSampleTracker.impl.hh>

#include <cmath>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(FloatTimeSampleTracker);
GM_OFI_PARAM2(FloatTimeSampleTracker, key, std::string, addKey);
GM_OFI_PARAM2(FloatTimeSampleTracker, value, float, addValue);
GM_OFI_PARAM2(FloatTimeSampleTracker, time, double, addTime);

template<>
float TimeSampleTracker<float>::Impl::interpolate(float a, float b, float r) {
  return std::lerp(a, b, r);
}

template class TimeSampleTracker<float>;

END_NAMESPACE_GMTRACK;
