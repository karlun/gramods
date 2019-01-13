
#include <gmNetwork/SyncData.hh>
#include <gmNetwork/SimpleDataSynchronization.hh>

BEGIN_NAMESPACE_GMNETWORK;

struct SyncData::Impl {

  std::weak_ptr<SimpleDataSynchronization> data_synchronizer;

};

SyncData::SyncData()
  : _impl(std::make_unique<Impl>()) {}

void SyncData::pushValue() {

  std::shared_ptr<SimpleDataSynchronization> sync = _impl->data_synchronizer.lock();
  if (!sync)
    return;

  sync->send(this);
}

void SyncData::setSynchronizer(std::shared_ptr<SimpleDataSynchronization> sync) {
  if (_impl->data_synchronizer.lock())
    throw std::invalid_argument("Cannot use SyncData in more than one synchronizer");

  _impl->data_synchronizer = sync;
}

END_NAMESPACE_GMNETWORK;
