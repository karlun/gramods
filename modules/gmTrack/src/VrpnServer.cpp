
#include <gmTrack/VrpnServer.hh>

#ifdef gramods_ENABLE_VRPN

#include <gmTrack/SinglePoseTracker.hh>
#include <gmTrack/MultiPoseTracker.hh>

#include <gmCore/Console.hh>

#include <vrpn_Tracker.h>
#include <vrpn_Analog.h>
#include <vrpn_Button.h>

#include <memory>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(VrpnServer);
GM_OFI_PARAM2(VrpnServer, port, int, setPort);
GM_OFI_PARAM2(VrpnServer, trackerName, std::string, addTrackerName);
GM_OFI_POINTER2(VrpnServer, analogsTracker, AnalogsTracker, addAnalogsTracker);
GM_OFI_POINTER2(VrpnServer, buttonsTracker, ButtonsTracker, addButtonsTracker);
GM_OFI_POINTER2(VrpnServer, multiPoseTracker, MultiPoseTracker, addMultiPoseTracker);
GM_OFI_POINTER2(VrpnServer, singlePoseTracker, SinglePoseTracker, addSinglePoseTracker);

#define ANALOG_IDX      0
#define BUTTON_IDX      1
#define MULTI_POSE_IDX  2
#define SINGLE_POSE_IDX 3

struct VrpnServer::Impl {

  Impl();
  ~Impl();

  void initialize();

  void updateAnalogs();
  void updateButtons();
  void updateMultiPose();
  void updateSinglePose();

  static std::map<int, vrpn_Connection*> vrpn_connections;
  std::vector<std::shared_ptr<vrpn_Tracker_Server>> single_tracker_server;
  std::vector<std::shared_ptr<vrpn_Tracker_Server>> multi_tracker_server;
  std::vector<std::shared_ptr<vrpn_Analog_Server>> analog_server;
  std::vector<std::shared_ptr<vrpn_Button_Server>> button_server;

  int port = vrpn_DEFAULT_LISTEN_PORT_NO;
  std::vector<std::string> tracker_names;
  std::vector<int> name_map;
  std::vector<std::shared_ptr<AnalogsTracker>> analogs_trackers;
  std::vector<std::shared_ptr<ButtonsTracker>> buttons_trackers;
  std::vector<std::shared_ptr<SinglePoseTracker>> single_pose_trackers;
  std::vector<std::shared_ptr<MultiPoseTracker>> multi_pose_trackers;
};

std::map<int, vrpn_Connection*> VrpnServer::Impl::vrpn_connections;

VrpnServer::VrpnServer()
  : gmCore::Updateable(-100),
  _impl(std::make_unique<Impl>()) {}

VrpnServer::~VrpnServer() {}

VrpnServer::Impl::Impl() {}

VrpnServer::Impl::~Impl() {}

void VrpnServer::initialize() {

  if (isInitialized())
    return;

  gmCore::Object::initialize();
  _impl->initialize();
}

void VrpnServer::Impl::initialize() {

  if (vrpn_connections.count(port) == 0)
    vrpn_connections[port] = vrpn_create_server_connection(port);

  if (tracker_names.size() < name_map.size())
    throw gmCore::InvalidArgument("too few tracker tracker names for the current amount of trackers");
  if (tracker_names.size() > name_map.size())
    throw gmCore::InvalidArgument("more tracker names provided than trackers");

  for (size_t idx = 0; idx < name_map.size(); ++idx) {

    std::string name = tracker_names[idx];

    switch (name_map[idx]) {

    case ANALOG_IDX:
      analog_server.push_back
        (std::make_shared<vrpn_Analog_Server>(name.c_str(), vrpn_connections[port]));
      break;

    case BUTTON_IDX:
      button_server.push_back
        (std::make_shared<vrpn_Button_Server>(name.c_str(), vrpn_connections[port], 32));
      break;

    case MULTI_POSE_IDX:
      multi_tracker_server.push_back
        (std::make_shared<vrpn_Tracker_Server>(name.c_str(), vrpn_connections[port], 32));
      break;

    case SINGLE_POSE_IDX:
      single_tracker_server.push_back
        (std::make_shared<vrpn_Tracker_Server>(name.c_str(), vrpn_connections[port]));
      break;
    }
    
  }
}

void VrpnServer::update(gmCore::Updateable::clock::time_point, size_t) {

  if (!isInitialized())
    return;

  _impl->updateAnalogs();
  _impl->updateButtons();
  _impl->updateMultiPose();
  _impl->updateSinglePose();

  for (auto conn : _impl->vrpn_connections )
    conn.second->mainloop();
}

#  define TP_TO_MS(X)                                                          \
    (std::chrono::duration_cast<std::chrono::milliseconds>(                    \
         (X).time_since_epoch())                                               \
         .count())

void VrpnServer::Impl::updateAnalogs() {

  assert(analogs_trackers.size() == analog_server.size());

  for (size_t idx = 0; idx < analog_server.size(); ++idx) {

    AnalogsTracker::AnalogsSample sample;
    bool good = analogs_trackers[idx]->getAnalogs(sample);
    if (!good)
      continue;

    vrpn_float64 *data = analog_server[idx]->channels();
    for (auto a : sample.analogs)
      *(data++) = a;

    analog_server[idx]->report(vrpn_CONNECTION_LOW_LATENCY,
                               vrpn_MsecsTimeval(TP_TO_MS(sample.time)));
    analog_server[idx]->mainloop();
  }
}

void VrpnServer::Impl::updateButtons() {

  assert(buttons_trackers.size() == button_server.size());

  for (size_t idx = 0; idx < button_server.size(); ++idx) {

    ButtonsTracker::ButtonsSample sample;
    bool good = buttons_trackers[idx]->getButtons(sample);
    if (!good)
      continue;

    for (auto btn : sample.buttons)
      button_server[idx]->set_button(btn.first, btn.second ? 1 : 0);

    button_server[idx]->mainloop();
  }
}

void VrpnServer::Impl::updateSinglePose() {

  assert(single_pose_trackers.size() == single_tracker_server.size());

  for (size_t idx = 0; idx < single_tracker_server.size(); ++idx) {

    PoseTracker::PoseSample sample;
    bool good = single_pose_trackers[idx]->getPose(sample);
    if (!good)
      continue;

    auto time = vrpn_MsecsTimeval(TP_TO_MS(sample.time));
    vrpn_float64 pos[3] = { sample.position[0],
                            sample.position[1],
                            sample.position[2] };
    vrpn_float64 quat[4] = { sample.orientation.x(),
                             sample.orientation.y(),
                             sample.orientation.z(),
                             sample.orientation.w() };
    single_tracker_server[idx]->report_pose(0, time, pos, quat);
    single_tracker_server[idx]->mainloop();
  }
}

void VrpnServer::Impl::updateMultiPose() {

  assert(multi_pose_trackers.size() == multi_tracker_server.size());

  for (size_t idx = 0; idx < multi_tracker_server.size(); ++idx) {

    std::map<int, PoseTracker::PoseSample> samples;
    bool good = multi_pose_trackers[idx]->getPose(samples);
    if (!good)
      continue;

    for (auto sample : samples) {

      auto time = vrpn_MsecsTimeval(TP_TO_MS(sample.second.time));
      vrpn_float64 pos[3] = { sample.second.position[0],
                              sample.second.position[1],
                              sample.second.position[2] };
      vrpn_float64 quat[4] = { sample.second.orientation.x(),
                               sample.second.orientation.y(),
                               sample.second.orientation.z(),
                               sample.second.orientation.w() };
      multi_tracker_server[idx]->report_pose(sample.first, time, pos, quat);
      multi_tracker_server[idx]->mainloop();
    }
  }
}

void VrpnServer::setPort(int port) {

  if (isInitialized())
    throw std::logic_error("Setting port after initialization");

  _impl->port = port;
}

void VrpnServer::addTrackerName(std::string name) {
  if (isInitialized()) throw std::logic_error("Add tracker name after initialization");
  _impl->tracker_names.push_back(name);
}

void VrpnServer::addAnalogsTracker(std::shared_ptr<AnalogsTracker> t) {
  if (!t) throw gmCore::InvalidArgument("null not allowed");
  if (isInitialized()) throw std::logic_error("Add AnalogsTracker after initialization");
  _impl->name_map.push_back(ANALOG_IDX);
  _impl->analogs_trackers.push_back(t);
}

void VrpnServer::addButtonsTracker(std::shared_ptr<ButtonsTracker> t) {
  if (!t) throw gmCore::InvalidArgument("null not allowed");
  if (isInitialized()) throw std::logic_error("Add ButtonsTracker after initialization");
  _impl->name_map.push_back(BUTTON_IDX);
  _impl->buttons_trackers.push_back(t);
}

void VrpnServer::addMultiPoseTracker(std::shared_ptr<MultiPoseTracker> t) {
  if (!t) throw gmCore::InvalidArgument("null not allowed");
  if (isInitialized()) throw std::logic_error("Add MultiPoseTracker after initialization");
  _impl->name_map.push_back(MULTI_POSE_IDX);
  _impl->multi_pose_trackers.push_back(t);
}

void VrpnServer::addSinglePoseTracker(std::shared_ptr<SinglePoseTracker> t) {
  if (!t) throw gmCore::InvalidArgument("null not allowed");
  if (isInitialized()) throw std::logic_error("Add SinglePoseTracker after initialization");
  _impl->name_map.push_back(SINGLE_POSE_IDX);
  _impl->single_pose_trackers.push_back(t);
}

void VrpnServer::traverse(Visitor *visitor) {
  for (auto &t : _impl->single_pose_trackers) t->accept(visitor);
  for (auto &t : _impl->multi_pose_trackers) t->accept(visitor);
  for (auto &t : _impl->buttons_trackers) t->accept(visitor);
  for (auto &t : _impl->analogs_trackers) t->accept(visitor);
}

END_NAMESPACE_GMTRACK;

#endif
