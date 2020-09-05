
#include <gmNetwork/SyncData.hh>
#include <gmNetwork/DataSync.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/InvalidArgument.hh>

BEGIN_NAMESPACE_GMNETWORK;

struct SyncData::Impl {

  std::weak_ptr<DataSync> data_synchronizer;

};

SyncData::SyncData()
  : _impl(std::make_unique<Impl>()) {}

void SyncData::pushValue() {

  std::shared_ptr<DataSync> sync = _impl->data_synchronizer.lock();
  if (sync)
    sync->send(this);
  else
    GM_RUNONCE(GM_WRN("SyncData", "Data not connected to any existing DataSync instance."));
}

void SyncData::setSynchronizer(std::shared_ptr<DataSync> sync) {
  if (_impl->data_synchronizer.lock())
    throw gmCore::InvalidArgument("Cannot use SyncData in more than one synchronizer.");

  _impl->data_synchronizer = sync;
}

END_NAMESPACE_GMNETWORK;
