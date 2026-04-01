
#include <gmTrack/PoseKeyChangeTracker.hh>
#include <gmTrack/KeyChangeTracker.impl.hh>

#include <gmCore/io_eigen.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(PoseKeyChangeTracker);
GM_OFI_PARAM2(PoseKeyChangeTracker, mapping, gmCore::string2, addMapping);
GM_OFI_POINTER2(PoseKeyChangeTracker, poseTracker, gmTrack::PoseTracker, addTracker);

template class KeyChangeTracker<gmCore::Pose>;

END_NAMESPACE_GMTRACK;
