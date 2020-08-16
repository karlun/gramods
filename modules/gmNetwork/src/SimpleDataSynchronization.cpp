
#include <gmNetwork/SimpleDataSynchronization.hh>

#include <gmNetwork/PeersConnection.hh>
#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMNETWORK;

GM_OFI_DEFINE_SUB(SimpleDataSynchronization, Protocol);

struct SimpleDataSynchronization::Impl {

  void addData(std::shared_ptr<SyncData> d);
  void addData(SyncData * d);

  void update();

  void encode(SyncData * d, std::vector<char> &data, int local_peer_idx);
  void processMessage(Message m, int local_peer_idx);

  bool closing = false;
  std::vector<std::shared_ptr<SyncData>> ptr_data;
  std::vector<SyncData*> raw_data;

};

SimpleDataSynchronization::SimpleDataSynchronization()
  : _impl(std::make_unique<Impl>()) {}

SimpleDataSynchronization::~SimpleDataSynchronization() {}

void SimpleDataSynchronization::addData(std::shared_ptr<SyncData> d) {
  if (!d)
    throw std::invalid_argument("Attempting to add data by null pointer");

  auto _this = std::static_pointer_cast<SimpleDataSynchronization>(this->shared_from_this());
  d->setSynchronizer(_this);

  _impl->addData(d);
}

void SimpleDataSynchronization::Impl::addData(std::shared_ptr<SyncData> d) {
  ptr_data.push_back(d);
}

void SimpleDataSynchronization::addData(SyncData * d) {
  if (!d)
    throw std::invalid_argument("Attempting to add data by null pointer");

  auto _this = std::static_pointer_cast<SimpleDataSynchronization>(this->shared_from_this());
  d->setSynchronizer(_this);

  _impl->addData(d);
}

void SimpleDataSynchronization::Impl::addData(SyncData * d) {
  raw_data.push_back(d);
}

void SimpleDataSynchronization::update() {
  _impl->update();
}

void SimpleDataSynchronization::Impl::update() {

  for (auto d : ptr_data)
    d->update();

  for (auto d : raw_data)
    d->update();
}

void SimpleDataSynchronization::processMessage(Message m) {
  _impl->processMessage(m, getConnection()->getLocalPeerIdx());
}

void SimpleDataSynchronization::Impl::processMessage(Message m, int local_peer_idx) {
  assert(m.data.size() > 1);

  size_t idx = (size_t)m.data[0];

  if (idx >= (ptr_data.size() + raw_data.size())) {
    GM_ERR("SimpleDataSynchronization",
           "Wrong data index - peers may not agree on data to synchronize");
    return;
  }

  if (idx < ptr_data.size()) {
    GM_VVINF("SimpleDataSynchronization",
             "Incoming data (" << m.peer_idx
             << " -> " << local_peer_idx
             << ") for ptr cell " << int(idx));
    ptr_data[idx]->decode(m.data);
  } else {
    GM_VVINF("SimpleDataSynchronization",
             "Incoming data (" << m.peer_idx
             << " -> " << local_peer_idx
             << ") for raw cell " << int(idx - ptr_data.size()));
    raw_data[idx - ptr_data.size()]->decode(m.data);
  }
}

void SimpleDataSynchronization::send(SyncData * d) {

  std::vector<char> data;
  _impl->encode(d, data, getConnection()->getLocalPeerIdx());

  sendMessage(data);
}

void SimpleDataSynchronization::Impl::encode(SyncData * d,
                                             std::vector<char> &data,
                                             int local_peer_idx) {
  d->encode(data);

  assert(data.size() > 1);

  for (size_t idx = 0; idx < ptr_data.size(); ++idx)
    if (ptr_data[idx].get() == d) {
      assert(idx < 256);
      data[0] = (char)idx;

      GM_VVINF("SimpleDataSynchronization",
               "Sending data (" << local_peer_idx
               << " -> all) for ptr cell " << int(idx));
      return;
    }

  for (size_t idx = 0; idx < raw_data.size(); ++idx)
    if (raw_data[idx] == d) {
      assert(idx + ptr_data.size() < 256);
      data[0] = (char)(idx + ptr_data.size());

      GM_VVINF("SimpleDataSynchronization",
               "Sending data (" << local_peer_idx
               << " -> all) for raw cell " << int(idx));
      return;
    }
  assert(0);
}

END_NAMESPACE_GMNETWORK;
