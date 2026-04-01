
#include <gmTrack/BinaryKeyChangeTracker.hh>
#include <gmTrack/KeyChangeTracker.impl.hh>

#include <gmCore/io_eigen.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(BinaryKeyChangeTracker);
GM_OFI_PARAM2(BinaryKeyChangeTracker, mapping, gmCore::string2, addMapping);
GM_OFI_POINTER2(BinaryKeyChangeTracker, BinaryTracker, gmTrack::BinaryTracker, addTracker);

template class KeyChangeTracker<bool>;

END_NAMESPACE_GMTRACK;
