
#include <gmTrack/Float2KeyChangeTracker.hh>
#include <gmTrack/KeyChangeTracker.impl.hh>

#include <gmCore/io_eigen.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(Float2KeyChangeTracker);
GM_OFI_PARAM2(Float2KeyChangeTracker, mapping, gmCore::string2, addMapping);
GM_OFI_POINTER2(Float2KeyChangeTracker, Float2Tracker, gmTrack::Float2Tracker, addTracker);

template class KeyChangeTracker<gmCore::float2>;

END_NAMESPACE_GMTRACK;
