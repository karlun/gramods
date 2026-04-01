
#include <gmTrack/VrpnServer.hh>

#ifdef gramods_ENABLE_VRPN

#include <gmCore/Console.hh>
#include <gmCore/Updateable.hh>

#include <vrpn_Tracker.h>
#include <vrpn_Analog.h>
#include <vrpn_Button.h>

#include <memory>
#include <regex>
#include <unordered_map>
#include <unordered_set>

using namespace std::literals::string_literals;

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(VrpnServer);
GM_OFI_PARAM2(VrpnServer, port, int, setPort);
GM_OFI_POINTER2(VrpnServer, binaryTracker, BinaryTracker, addBinaryTracker);
GM_OFI_POINTER2(VrpnServer, floatTracker, FloatTracker, addFloatTracker);
GM_OFI_POINTER2(VrpnServer, float2Tracker, Float2Tracker, addFloat2Tracker);
GM_OFI_POINTER2(VrpnServer, poseTracker, PoseTracker, addPoseTracker);

struct VrpnServer::Impl : public gmCore::Updateable {

  Impl();
  ~Impl();

  void initialize();
  void update(gmCore::Updateable::clock::time_point time,
              size_t frame) override;

  void updateBinary();
  void updateFloat();
  void updateFloat2();
  void updatePose();

  std::unordered_map<std::string, vrpn_int32> key_mapping;

  static std::map<int, vrpn_Connection *> vrpn_connections;
  std::unordered_map<std::string, std::shared_ptr<vrpn_Button_Server>>
      button_servers;
  std::unordered_map<std::string, std::shared_ptr<vrpn_Analog_Server>>
      analog_servers;
  std::unordered_map<std::string, std::shared_ptr<vrpn_Analog_Server>>
      analog2_servers;
  std::unordered_map<std::string, std::shared_ptr<vrpn_Tracker_Server>>
      tracker_servers;

  int port = vrpn_DEFAULT_LISTEN_PORT_NO;
  std::vector<std::shared_ptr<BinaryTracker>> binary_trackers;
  std::vector<std::shared_ptr<FloatTracker>> float_trackers;
  std::vector<std::shared_ptr<Float2Tracker>> float2_trackers;
  std::vector<std::shared_ptr<PoseTracker>> pose_trackers;
};

std::map<int, vrpn_Connection*> VrpnServer::Impl::vrpn_connections;

VrpnServer::VrpnServer() : _impl(std::make_unique<Impl>()) {}

VrpnServer::~VrpnServer() {}

VrpnServer::Impl::Impl() : gmCore::Updateable(-100) {}

VrpnServer::Impl::~Impl() {
  if (vrpn_connections.contains(port))
    vrpn_connections[port]->removeReference();
}

void VrpnServer::initialize() {
  if (isInitialized())
    throw std::logic_error("Initialized called more than once");
  _impl->initialize();
}

void VrpnServer::Impl::initialize() {
  if (!vrpn_connections.contains(port))
    vrpn_connections[port] = vrpn_create_server_connection(port);
}

void VrpnServer::Impl::update(gmCore::Updateable::clock::time_point, size_t) {

  updateBinary();
  updateFloat();
  updateFloat2();
  updatePose();

  for (auto conn : vrpn_connections) conn.second->mainloop();
}

#define TP_TO_MS(X)                                                            \
  (std::chrono::duration_cast<std::chrono::milliseconds>(                      \
       (X).time_since_epoch())                                                 \
       .count())

namespace {
std::tuple<std::string, std::optional<int>> get_key_sensor(std::string path) {
  static const std::regex re_w_channel //
      (R"(\s*/?(.*)/([0-9]+))", std::regex_constants::ECMAScript);
  static const std::regex re_no_channel //
      (R"(\s*/?(.*)/?\s*)", std::regex_constants::ECMAScript);

  std::smatch m;
  if (std::regex_match(path, m, re_w_channel)) {
    return {m[1].str(), std::atoi(m[2].str().c_str())};
  } else if (std::regex_match(path, m, re_no_channel)) {
    return {m[1].str(), std::nullopt};
  } else {
    return {path, std::nullopt};
  }
}
}

void VrpnServer::Impl::updateBinary() {

  std::unordered_set<vrpn_Button_Server*> updated_servers;

  for (auto tracker : binary_trackers) {
    auto state = tracker->get();
    if (!state) continue;

    for (auto as : state.value()) {
      const auto [key, sensor] = get_key_sensor(as.first);
      if (!button_servers.contains(key)) {
        GM_DBG2("VrpnServer", "New vrpn_Button_Server " << key);
        button_servers[key] = std::make_shared<vrpn_Button_Server>(
            key.c_str(), vrpn_connections[port]);
      }
      button_servers[key]->set_button(sensor.value_or(0), as.second.value);
      updated_servers.insert(button_servers[key].get());
    }
  }

  for (auto server : updated_servers) server->mainloop();
}

void VrpnServer::Impl::updateFloat() {

  std::unordered_set<vrpn_Analog_Server*> updated_servers;

  for (auto tracker : float_trackers) {
    auto state = tracker->get();
    if (!state) continue;

    for (auto as : state.value()) {
      const auto [key, sensor] = get_key_sensor(as.first);
      const auto channel = sensor.value_or(0);

      if (!analog_servers.contains(key)) {
        GM_DBG2("VrpnServer", "New vrpn_Analog_Server " << key);
        analog_servers[key] = std::make_shared<vrpn_Analog_Server>(
            key.c_str(), vrpn_connections[port]);
      }
      analog_servers[key]->channels()[channel] = as.second.value;
      analog_servers[key]->report(vrpn_CONNECTION_LOW_LATENCY,
                                  vrpn_MsecsTimeval(TP_TO_MS(as.second.time)));

      if (channel + 1 > analog_servers[key]->getNumChannels())
        analog_servers[key]->setNumChannels(channel + 1);
      updated_servers.insert(analog_servers[key].get());
    }
  }

  for (auto server : updated_servers) server->mainloop();
}

void VrpnServer::Impl::updateFloat2() {

  std::unordered_set<vrpn_Analog_Server*> updated_servers;

  for (auto tracker : float2_trackers) {
    auto state = tracker->get();
    if (!state) continue;

    for (auto as : state.value()) {
      const auto [key, sensor] = get_key_sensor(as.first);

      const auto channel_x = sensor ? *sensor : 0;
      const auto channel_y = sensor ? *sensor : 1;
      const auto key_x = sensor ? (key + "/x") : key;
      const auto key_y = sensor ? (key + "/y") : key;

      if (!analog2_servers.contains(key_x)) {
        GM_DBG2("VrpnServer", "New vrpn_Analog_Server " << key);
        analog2_servers.insert({key_x,
                                std::make_shared<vrpn_Analog_Server>(
                                    key_x.c_str(), vrpn_connections[port])});
      }
      analog2_servers[key_x]->channels()[channel_x] = as.second.value[0];

      if (channel_x + 1 > analog2_servers[key_x]->getNumChannels())
        analog2_servers[key_x]->setNumChannels(channel_x + 1);
      updated_servers.insert(analog2_servers[key_x].get());

      if (!analog2_servers.contains(key_y)) {
        GM_DBG2("VrpnServer", "New vrpn_Analog_Server " << key);
        analog2_servers.insert({key_y,
                                std::make_shared<vrpn_Analog_Server>(
                                    key_y.c_str(), vrpn_connections[port])});
      }
      analog2_servers[key_y]->channels()[channel_y] = as.second.value[1];

      if (channel_y + 1 > analog2_servers[key_y]->getNumChannels())
        analog2_servers[key_y]->setNumChannels(channel_y + 1);
      updated_servers.insert(analog2_servers[key_y].get());
    }
  }

  for (auto server : updated_servers) server->mainloop();
}

void VrpnServer::Impl::updatePose() {

  std::unordered_set<vrpn_Tracker_Server*> updated_servers;

  for (auto tracker : pose_trackers) {
    auto state = tracker->get();
    if (!state) continue;

    for (auto as : state.value()) {
      const auto [key, sensor] = get_key_sensor(as.first);
      if (!tracker_servers.contains(key)) {
        GM_DBG2("VrpnServer", "New vrpn_Tracker_Server " << key);
        tracker_servers[key] = std::make_shared<vrpn_Tracker_Server>(
            key.c_str(), vrpn_connections[port]);
      }

      vrpn_float64 pos[3] = {as.second.value.position.x(), //
                             as.second.value.position.y(),
                             as.second.value.position.z()};
      vrpn_float64 quat[4] = {as.second.value.orientation.x(),
                              as.second.value.orientation.y(),
                              as.second.value.orientation.z(),
                              as.second.value.orientation.w()};

      auto time = vrpn_MsecsTimeval(TP_TO_MS(as.second.time));
      tracker_servers[key]->report_pose(sensor.value_or(0), time, pos, quat);
      updated_servers.insert(tracker_servers[key].get());
    }
  }

  for (auto server : updated_servers) server->mainloop();
}

void VrpnServer::setPort(int port) {

  if (isInitialized())
    throw std::logic_error("Setting port after initialization");

  _impl->port = port;
}

void VrpnServer::addBinaryTracker(std::shared_ptr<BinaryTracker> t) {
  if (!t) throw gmCore::InvalidArgument("null not allowed");
  if (isInitialized()) throw std::logic_error("Add BinaryTracker after initialization");
  _impl->binary_trackers.push_back(t);
}

void VrpnServer::addFloatTracker(std::shared_ptr<FloatTracker> t) {
  if (!t) throw gmCore::InvalidArgument("null not allowed");
  if (isInitialized()) throw std::logic_error("Add FloatTracker after initialization");
  _impl->float_trackers.push_back(t);
}

void VrpnServer::addFloat2Tracker(std::shared_ptr<Float2Tracker> t) {
  if (!t) throw gmCore::InvalidArgument("null not allowed");
  if (isInitialized()) throw std::logic_error("Add Float2Tracker after initialization");
  _impl->float2_trackers.push_back(t);
}

void VrpnServer::addPoseTracker(std::shared_ptr<PoseTracker> t) {
  if (!t) throw gmCore::InvalidArgument("null not allowed");
  if (isInitialized()) throw std::logic_error("Add PoseTracker after initialization");
  _impl->pose_trackers.push_back(t);
}

void VrpnServer::traverse(Visitor *visitor) {
  for (auto &t : _impl->binary_trackers) t->accept(visitor);
  for (auto &t : _impl->float_trackers) t->accept(visitor);
  for (auto &t : _impl->float2_trackers) t->accept(visitor);
  for (auto &t : _impl->pose_trackers) t->accept(visitor);
}

END_NAMESPACE_GMTRACK;

#endif
