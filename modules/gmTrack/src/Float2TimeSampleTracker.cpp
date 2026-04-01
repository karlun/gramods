
#include <gmCore/io_float.hh>

#include <gmTrack/Float2TimeSampleTracker.hh>
#include <gmTrack/TimeSampleTracker.impl.hh>

#include <cmath>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(Float2TimeSampleTracker);
GM_OFI_PARAM2(Float2TimeSampleTracker, key, std::string, addKey);
GM_OFI_PARAM2(Float2TimeSampleTracker, value, gmCore::float2, addValue);
GM_OFI_PARAM2(Float2TimeSampleTracker, time, double, addTime);

template<>
gmCore::float2 Float2TimeSampleTracker::Impl::interpolate(gmCore::float2 a,
                                                          gmCore::float2 b,
                                                          float r) {
  return {std::lerp(a[0], b[0], r), std::lerp(a[1], b[1], r)};
}

template class TimeSampleTracker<gmCore::float2>;

END_NAMESPACE_GMTRACK;
