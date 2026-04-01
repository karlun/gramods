
#include <gmTrack/FloatKeyChangeTracker.hh>
#include <gmTrack/KeyChangeTracker.impl.hh>

#include <gmCore/io_eigen.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(FloatKeyChangeTracker);
GM_OFI_PARAM2(FloatKeyChangeTracker, mapping, gmCore::string2, addMapping);
GM_OFI_POINTER2(FloatKeyChangeTracker, FloatTracker, gmTrack::FloatTracker, addTracker);

template class KeyChangeTracker<float>;

END_NAMESPACE_GMTRACK;
