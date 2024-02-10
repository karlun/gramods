
#include <gmNetwork/DataSync.hh>

#include <gmNetwork/SyncNode.hh>

#include <gmCore/InvalidArgument.hh>
#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMNETWORK;

struct DataSync::Impl {

  void addData(std::shared_ptr<SyncData> d);
  void addData(SyncData * d);

  void update();

  void encode(SyncData * d, std::vector<char> &data, size_t local_peer_idx);
  void processMessage(Message m, size_t local_peer_idx);

  bool closing = false;
  std::vector<std::shared_ptr<SyncData>> ptr_data;
  std::vector<SyncData*> raw_data;

  std::mutex impl_lock;
};

DataSync::DataSync()
  : _impl(std::make_unique<Impl>()) {}

DataSync::~DataSync() {}

void DataSync::addData(std::shared_ptr<SyncData> d) {
  if (!d)
    throw gmCore::InvalidArgument("Attempting to add data by null pointer");

  auto _this = std::static_pointer_cast<DataSync>(this->shared_from_this());
  d->setSynchronizer(_this);

  _impl->addData(d);
}

void DataSync::Impl::addData(std::shared_ptr<SyncData> d) {
  std::lock_guard<std::mutex> guard(impl_lock);
  ptr_data.push_back(d);
}

void DataSync::addData(SyncData * d) {
  if (!d)
    throw gmCore::InvalidArgument("Attempting to add data by null pointer");

  auto _this = std::static_pointer_cast<DataSync>(this->shared_from_this());
  d->setSynchronizer(_this);

  _impl->addData(d);
}

void DataSync::Impl::addData(SyncData * d) {
  std::lock_guard<std::mutex> guard(impl_lock);
  raw_data.push_back(d);
}

void DataSync::update() {
  _impl->update();
}

void DataSync::Impl::update() {

  std::lock_guard<std::mutex> guard(impl_lock);

  for (auto d : ptr_data)
    d->update();

  for (auto d : raw_data)
    d->update();
}

void DataSync::processMessage(Message m) {
  _impl->processMessage(m, getLocalPeerIdx());
}

void DataSync::Impl::processMessage(Message m, size_t local_peer_idx) {
  assert(m.data.size() > 1);

  size_t idx = (size_t)m.data[0];

  if (idx >= (ptr_data.size() + raw_data.size())) {
    GM_ERR("DataSync",
           "Wrong data index ("
               << idx << " >= " << (ptr_data.size() + raw_data.size())
               << ") - peers may not agree on data to synchronize");
    return;
  }

  if (idx < ptr_data.size()) {
    GM_DBG3("DataSync",
             "Incoming data (" << m.from_peer_idx
             << " -> " << local_peer_idx
             << ") for ptr cell " << idx);
    ptr_data[idx]->decode(m.data);
  } else {
    GM_DBG3("DataSync",
             "Incoming data (" << m.from_peer_idx
             << " -> " << local_peer_idx
             << ") for raw cell " << idx - ptr_data.size());
    raw_data[idx - ptr_data.size()]->decode(m.data);
  }
}

void DataSync::send(SyncData * d) {

  std::vector<char> data;
  _impl->encode(d, data, getLocalPeerIdx());

  sendMessage(data);
}

void DataSync::Impl::encode(SyncData * d,
                            std::vector<char> &data,
                            size_t local_peer_idx) {
  d->encode(data);

  assert(data.size() > 1);

  for (size_t idx = 0; idx < ptr_data.size(); ++idx)
    if (ptr_data[idx].get() == d) {
      assert(idx < 256);
      data[0] = (char)idx;

      GM_DBG3("DataSync",
               "Sending data (" << local_peer_idx
               << " -> all) for ptr cell " << idx);
      return;
    }

  for (size_t idx = 0; idx < raw_data.size(); ++idx)
    if (raw_data[idx] == d) {
      assert(idx + ptr_data.size() < 256);
      data[0] = (char)(idx + ptr_data.size());

      GM_DBG3("DataSync",
               "Sending data (" << local_peer_idx
               << " -> all) for raw cell " << idx);
      return;
    }
  assert(0);
}

END_NAMESPACE_GMNETWORK;
