
#include <gmTouch/TouchState.hh>
#include <gmCore/InvalidArgument.hh>

#include <stdexcept>
#include <limits>

BEGIN_NAMESPACE_GMTOUCH;

#define DEFAULT_SMOOTHING 0.2f
#define DEFAULT_MOVE_MAGNITUDE 10
#define DEFAULT_MOVE_HISTORY_LENGTH 100
#define DEFAULT_MOVE_HISTORY_DURATION 1.f
#define DEFAULT_HOLD_DELAY_MS 2000
#define DEFAULT_CLICK_DELAY_MS 500
#define DEFAULT_MULTI_DELAY_MS 500
#define DEFAULT_NOISE_LEVEL 5
#define DEFAULT_VELOCITY_REESTIMATION_RATE 3.f

#define MUST_BE_CALLED(METH) #METH "must be called between eventsInit and eventsDone"
#define CANNOT_BE_CALLED(METH) #METH "can not be called between eventsInit and eventsDone"

const TouchState::TouchPointId TouchState::MOUSE_STATE_ID = std::numeric_limits<TouchPointId>::max();

TouchState::TouchState()
  : start_time(clock::now()),
    noise_level(DEFAULT_NOISE_LEVEL),
    velocity_reestimation_rate(DEFAULT_VELOCITY_REESTIMATION_RATE),
    smoothing(DEFAULT_SMOOTHING),
    move_magnitude(DEFAULT_MOVE_MAGNITUDE),
    hold_time(std::chrono::milliseconds(DEFAULT_HOLD_DELAY_MS)),
    click_time(std::chrono::milliseconds(DEFAULT_CLICK_DELAY_MS)),
    multi_time(std::chrono::milliseconds(DEFAULT_MULTI_DELAY_MS)) {

  velocityEstimator.setHistoryLength(DEFAULT_MOVE_HISTORY_LENGTH);
  velocityEstimator.setHistoryDuration(DEFAULT_MOVE_HISTORY_DURATION);
}

TouchState::~TouchState() {
  for (auto it : event_adaptors)
    delete it.second;
  for (auto it : camera_adaptors)
    delete it.second;
}

int TouchState::getTouchPoints(TouchPoints &current) const {
  if (state != 0) throw std::logic_error(CANNOT_BE_CALLED(getTouchPoints));
  
  current.clear();
  current.reserve(current_state.size());
  
  for (auto tp : current_state)
    current.push_back(tp.second);
  
  return current.size();
}

int TouchState::getTouchPoints(TouchPoints &current, TouchPoints &previous) const {
  if (state != 0) throw std::logic_error(CANNOT_BE_CALLED(getTouchPoints));
  
  current.clear();
  previous.clear();
  
  current.reserve(current_state.size());
  previous.reserve(previous_state.size());
  
  for (auto tp : current_state) {
    if (previous_state.find(tp.second.id) == previous_state.end()) continue;
    current.push_back(tp.second);
    previous.push_back(previous_state.find(tp.second.id)->second);
  }
  
  return current.size();
}

int TouchState::getTouchPoints(void *ass, TouchPoints &current) const {
  std::map<void*, TouchPoints> curr;
  getTouchPoints(curr);
  current.swap(curr[ass]);
  return current.size();
}

int TouchState::getTouchPoints(void *ass, TouchPoints &current, TouchPoints &previous) const {
  std::map<void*, TouchPoints> curr;
  std::map<void*, TouchPoints> prev;
  getTouchPoints(curr, prev);
  current.swap(curr[ass]);
  previous.swap(prev[ass]);
  return current.size();
  
}

int TouchState::getTouchPoints(std::map<void*, TouchPoints> &current) const {
  if (state != 0) throw std::logic_error(CANNOT_BE_CALLED(getTouchPoints));

  current.clear();

  std::map<TouchPointId, TouchPoint>::const_iterator tp_it = current_state.begin();
  std::map<TouchPointId, void*>::const_iterator ass_it = current_association.begin();

  while (tp_it != current_state.end() && ass_it != current_association.end()) {

    if (tp_it->first < ass_it->first) {
      current[nullptr].push_back(tp_it->second);
      ++tp_it;
    } else if (tp_it->first > ass_it->first) {
      ++ass_it;
    } else {

      // Make a *released* copy of recently re-associated points
      if (last_association.find(tp_it->first) != last_association.end() &&
          last_association.find(tp_it->first)->second != ass_it->second) {
        current[last_association.find(tp_it->first)->second].push_back(tp_it->second);
        current[last_association.find(tp_it->first)->second].back().state |= State::RELEASE;
      }

      current[ass_it->second].push_back(tp_it->second);

      ++tp_it;
      ++ass_it;
    }
  }

  for (; tp_it != current_state.end(); ++tp_it) {

    // Make a *released* copy of recently un-associated points
    if (last_association.find(tp_it->first) != last_association.end()) {
      current[last_association.find(tp_it->first)->second].push_back(tp_it->second);
      current[last_association.find(tp_it->first)->second].back().state |= State::RELEASE;
    }

    current[nullptr].push_back(tp_it->second);
  }

  return current.size();
}

int TouchState::getTouchPoints(std::map<void*, TouchPoints> &current,
                               std::map<void*, TouchPoints> &previous) const {
  if (state != 0) throw std::logic_error(CANNOT_BE_CALLED(getTouchPoints));
  
  current.clear();
  previous.clear();

  std::map<TouchPointId, TouchPoint>::const_iterator tp_it = current_state.begin();
  std::map<TouchPointId, void*>::const_iterator ass_it = current_association.begin();

  while (tp_it != current_state.end() && ass_it != current_association.end()) {
    if (tp_it->first < ass_it->first) {
      
      if (previous_state.find(tp_it->second.id) != previous_state.end()) {
        current[nullptr].push_back(tp_it->second);
        previous[nullptr].push_back(previous_state.find(tp_it->second.id)->second);
      }
      
      ++tp_it;
    } else if (tp_it->first > ass_it->first) {
      ++ass_it;
    } else {

      if (previous_state.find(tp_it->second.id) != previous_state.end()) {
        current[ass_it->second].push_back(tp_it->second);
        previous[ass_it->second].push_back(previous_state.find(tp_it->second.id)->second);
      }
      
      ++tp_it;
      ++ass_it;
    }
  }
  for (; tp_it != current_state.end(); ++tp_it) {
    if (previous_state.find(tp_it->second.id) == previous_state.end())
      continue;
    current[nullptr].push_back(tp_it->second);
    previous[nullptr].push_back(previous_state.find(tp_it->second.id)->second);
  }
  
  assert(current.size() == previous.size());
  return current.size();
}


bool TouchState::setAssociation(TouchPointId id, void* pt) {
  if (state != 0) throw std::logic_error(CANNOT_BE_CALLED(setAssociation));
  if (pt == nullptr) throw gmCore::InvalidArgument("pt == nullptr");
  if (current_state.count(id) == 0) return false;
  current_association[id] = pt;
  return true;
}

bool TouchState::unsetAssociation(TouchPointId id, void* pt) {
  if (state != 0) throw std::logic_error(CANNOT_BE_CALLED(unsetAssociation));
  if (current_association.count(id) != 1) return false;
  if (current_association[id] != pt) return false;
  current_association.erase(id);
  return true;
}

bool TouchState::getAssociation(TouchPointId id, void** pt) const {
  if (state != 0) throw std::logic_error(CANNOT_BE_CALLED(getAssociation));
  if (current_association.find(id) == current_association.end()) return false;
  if (pt) *pt = current_association.find(id)->second;
  return true;
}


void TouchState::setCurrentProjection(Eigen::Matrix4f WPV_inv) {
  if (state != 1) throw std::logic_error(MUST_BE_CALLED(setCurrentProjection));
  current_WPV_inv = WPV_inv;
  current_WPV_inv_valid = true;
}

bool TouchState::getTouchLines(TouchLines &current) const {
  if (state != 0) throw std::logic_error(CANNOT_BE_CALLED(getTouchLines));
  if (!current_WPV_inv_valid) return false;
  
  current.clear();
  
  TouchPoints cur_pts;
  getTouchPoints(cur_pts);
  current.reserve(cur_pts.size());

  for (auto pt : cur_pts)
    current.push_back(touchPointToTouchLine(pt, current_WPV_inv));

  return true;
}

bool TouchState::getTouchLines(TouchLines &current, TouchLines &previous) const {
  if (state != 0) throw std::logic_error(CANNOT_BE_CALLED(getTouchLines));
  if (!current_WPV_inv_valid) return false;
  if (!previous_WPV_inv_valid) return false;

  current.clear();
  previous.clear();
  
  TouchPoints cur_pts, pre_pts;
  getTouchPoints(cur_pts, pre_pts);

  current.reserve(cur_pts.size());
  for (auto pt : cur_pts)
    current.push_back(touchPointToTouchLine(pt, current_WPV_inv));

  previous.reserve(pre_pts.size());
  for (auto pt : pre_pts)
    previous.push_back(touchPointToTouchLine(pt, previous_WPV_inv));
  
  return true;
}

bool TouchState::getTouchLines(void *ass, TouchLines &current) const {
  std::map<void*, TouchLines> ass_map;
  if (!getTouchLines(ass_map)) return false;
  current = ass_map[ass];
  return true;
}

bool TouchState::getTouchLines(std::map<void*, TouchLines> &current) const {
  if (state != 0) throw std::logic_error(CANNOT_BE_CALLED(getTouchLines));
  if (!current_WPV_inv_valid) return false;

  current.clear();
  
  std::map<void*, TouchPoints> cur_pts;
  getTouchPoints(cur_pts);
  
  for (auto it : cur_pts) {
    TouchLines lines;
    lines.reserve(it.second.size());
    
    for (auto pt : it.second)
      lines.push_back(touchPointToTouchLine(pt, current_WPV_inv));

    current[it.first].swap(lines);
  }
  
  return true;
}

bool TouchState::getTouchLines(void *ass, TouchLines &current, TouchLines &previous) const {
  std::map<void*, TouchLines> ass_map_cur, ass_map_pre;
  if (!getTouchLines(ass_map_cur, ass_map_pre)) return false;
  current = ass_map_cur[ass];
  previous = ass_map_pre[ass];
  return true;
}

bool TouchState::getTouchLines(std::map<void*, TouchLines> &current,
                               std::map<void*, TouchLines> &previous) const {
  if (state != 0) throw std::logic_error(CANNOT_BE_CALLED(getTouchLines));
  if (!current_WPV_inv_valid) return false;
  if (!previous_WPV_inv_valid) return false;

  current.clear();
  previous.clear();
  
  std::map<void*, TouchPoints> cur_pts;
  std::map<void*, TouchPoints> pre_pts;
  getTouchPoints(cur_pts, pre_pts);
  
  for (auto it : cur_pts) {
    TouchLines lines;
    lines.reserve(it.second.size());
    
    for (auto pt : it.second)
      lines.push_back(touchPointToTouchLine(pt, current_WPV_inv));

    current[it.first].swap(lines);
  }

  for (auto it : pre_pts) {
    TouchLines lines;
    lines.reserve(it.second.size());
    
    for (auto pt : it.second)
      lines.push_back(touchPointToTouchLine(pt, previous_WPV_inv));

    previous[it.first].swap(lines);
  }

  return true;
}


float TouchState::getMouseWheel() const {
  return mouse_wheel;
}

bool TouchState::getMouseDown() const {
  return mouse_down;
}

void TouchState::getMousePoint(int &x, int &y) const {
  x = mouse_point_x;
  y = mouse_point_y;
}

bool TouchState::getMouseLine(Eigen::Vector3f &x,
                              Eigen::Vector3f &v) const {
  if (! current_WPV_inv_valid) return false;

  int px, py;
  getMousePoint(px, py);
  
  Eigen::Vector4f nearPoint =
    current_WPV_inv * Eigen::Vector3f(px, current_height - py, 0.f).homogeneous();
  Eigen::Vector4f farPoint =
    current_WPV_inv * Eigen::Vector3f(px, current_height - py, 1.f).homogeneous();
  
  Eigen::Vector3f p0 = nearPoint.hnormalized();
  Eigen::Vector3f p1 = farPoint.hnormalized();

  x = p0;
  v = (p1 - p0).normalized();
  
  return true;
}

TouchState::TouchLine TouchState::touchPointToTouchLine(TouchPoint pt,
                                                        Eigen::Matrix4f) const {

  Eigen::Vector4f nearPoint =
    current_WPV_inv * Eigen::Vector3f(pt.sx, current_height - pt.sy, 0.f).homogeneous();
  Eigen::Vector4f farPoint =
    current_WPV_inv * Eigen::Vector3f(pt.sx, current_height - pt.sy, 1.f).homogeneous();

  Eigen::Vector3f p0 = nearPoint.hnormalized();
  Eigen::Vector3f p1 = farPoint.hnormalized();

  TouchLine tl = { p0, (p1 - p0).normalized(),
                   pt.id,
                   pt.state,
                   pt.clicks };

  return tl;
}

void TouchState::setSmoothing(float r) {
  if (r < 0.f) throw gmCore::InvalidArgument("r < 0");
  if (r >= 1.f) throw gmCore::InvalidArgument("r >= 1");
  smoothing = r;
}

float TouchState::getSmoothing() {
  return smoothing;
}

void TouchState::setRemoveMouseUponTouch(bool on) {
  remove_mouse_upon_touch = on;
}

bool TouchState::getRemoveMouseUponTouch() {
  return remove_mouse_upon_touch;
}

void TouchState::setMoveMagnitude(float dist) {
  move_magnitude = dist;
}

float TouchState::getMoveMagnitude() {
  return move_magnitude;
}

void TouchState::setHoldTime(clock::duration time) {
  hold_time = time;
}

TouchState::clock::duration TouchState::getHoldTime() {
  return hold_time;
}

void TouchState::setClickTime(clock::duration time) {
  click_time = time;
}

TouchState::clock::duration TouchState::getClickTime() {
  return click_time;
}

void TouchState::setMultiClickTime(clock::duration time) {
  multi_time = time;
}

TouchState::clock::duration TouchState::getMultiClickTime() {
  return multi_time;
}

void TouchState::EventAdaptor::init(int, int) {}

void TouchState::EventAdaptor::done() {}

void TouchState::EventAdaptor::addTouchState(TouchPointId id, float x, float y) {
  assert(owner);
  owner->addTouchState(id, x, y);
}

void TouchState::EventAdaptor::removeTouchState(TouchPointId id, float x, float y) {
  assert(owner);
  owner->removeTouchState(id, x, y);
}

void TouchState::EventAdaptor::addMouseState(float x, float y, bool down) {
  assert(owner);
  owner->addMouseState(x, y, down);
}

void TouchState::EventAdaptor::addMouseWheel(float s) {
  assert(owner);
  owner->addMouseWheel(s);
}

void TouchState::CameraAdaptor::init(int, int) {}

void TouchState::CameraAdaptor::done() {}

void TouchState::CameraAdaptor::setCurrentProjection(Eigen::Matrix4f WPV_inv) {
  assert(owner);
  owner->setCurrentProjection(WPV_inv);
}

void TouchState::eventsInit(int width, int height) {
  if (state != 0) throw std::logic_error("eventsInit and eventsDone must be called once each");
  state = 1;

  previous_state = current_state;
  
  previous_width = current_width;
  previous_height = current_height;
  current_width = width;
  current_height = height;

  previous_WPV_inv = current_WPV_inv;
  previous_WPV_inv_valid = current_WPV_inv_valid;
  
  for (auto it : event_adaptors)
    it.second->init(width, height);
  for (auto it : camera_adaptors)
    it.second->init(width, height);
  
  mouse_wheel = 0.f;

  last_association = current_association;
  clearReleasedStates();
}

void TouchState::addTouchState(TouchPointId id, float x, float y) {
  if (state != 1) throw std::logic_error(MUST_BE_CALLED(addTouchState));

  if (use_mouse && remove_mouse_upon_touch) {

    if (previous_state.find(MOUSE_STATE_ID) == previous_state.end() &&
        current_state.find(MOUSE_STATE_ID) != current_state.end())
      current_state.erase(MOUSE_STATE_ID);
    else
      addMouseState(x, y, false);

    mouse_down = false;
    use_mouse = false;
  }

  addState(id, x, y);
}

void TouchState::addState(TouchPointId id, float x, float y) {
  if (current_state.find(id) == current_state.end()) {
    TouchPoint p(id, x, y);
    current_state.insert(std::make_pair(id, p));
  } else {
    current_state[id].x = x;
    current_state[id].y = y;
  }

  double epoch =
      std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(
          clock::now() - start_time)
          .count();
  velocityEstimator.addSample(id, {x, y, 0}, epoch);
}

void TouchState::removeTouchState(TouchPointId id, float, float) {
  if (state != 1) throw std::logic_error(MUST_BE_CALLED(removeTouchState));

  if (current_state.find(id) == current_state.end()) return;
  
  current_state[id].state |= State::RELEASE;
}

void TouchState::addMouseState(float x, float y, bool down) {
  if (state != 1) throw std::logic_error(MUST_BE_CALLED(addMouseState));
  
  mouse_down = down;

  mouse_point_x = int(x);
  mouse_point_y = int(y);

  if (down) {
    if (!use_mouse) return;
    addState(MOUSE_STATE_ID, x, y);
  } else {
    if (current_state.find(MOUSE_STATE_ID) == current_state.end()) return;
    removeTouchState(MOUSE_STATE_ID, x, y);
  }
}

void TouchState::addMouseWheel(float s) {
  mouse_wheel = s;
}

void TouchState::eventsDone() {
  if (state != 1) throw std::logic_error("eventsInit and eventsDone must be called once each");

  for (auto it : event_adaptors)
    it.second->done();
  for (auto it : camera_adaptors)
    it.second->done();

  clock::time_point now = clock::now();
  double now_epoch =
      std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(
          now - start_time)
          .count();

  for (auto &pt : current_state) {
    
    double sec_last_time = velocityEstimator.getLastSampleTime(pt.first, 1);
    double last_time = velocityEstimator.getLastSampleTime(pt.first);

    bool added_temporary_sample = false;
    if (velocity_reestimation_rate > 1 && sec_last_time > -1 &&
        (now_epoch - last_time) >
            velocity_reestimation_rate * (last_time - sec_last_time)) {

      // It's been a while since last sample: assume that the finger
      // is stationary, so temporarily add a copy of the most recent
      // sample when estimating finger position and velocity.

      velocityEstimator.addSample(pt.first, {pt.second.x, pt.second.y, 0}, now_epoch);
      added_temporary_sample = true;
    }

    size_t samples;
    auto pos = velocityEstimator.estimatePosition(pt.first, noise_level, last_time, &samples);
    if (samples > 0) {
      pt.second.ex = float(pos[0]);
      pt.second.ey = float(pos[1]);
    } else {
      pt.second.ex = pt.second.x;
      pt.second.ey = pt.second.y;
    }

    auto vel = velocityEstimator.estimateVelocity(pt.first, noise_level, &samples);
    pt.second.vx = float(vel[0]);
    pt.second.vy = float(vel[1]);

    if (added_temporary_sample)
      velocityEstimator.removeLastSample(pt.first);

    if (history.find(pt.first) == history.end()) {

      // This is a new point, that has no history

      check_multi(pt.second);

      pt.second.state |= State::TOUCH_DOWN;

      HistoryState hs = { pt.second, now };
      history[pt.first] = hs;
      
      pt.second.sx = pt.second.x;
      pt.second.sy = pt.second.y;

    } else if (history[pt.first].point.state & State::RELEASE) {

      HistoryState hs = { pt.second, now };
      history[pt.first] = hs;

      pt.second.state &= ~State::TOUCH_DOWN;

      assert(previous_state.find(pt.first) != previous_state.end());
      TouchPoint old_pt = previous_state.find(pt.first)->second;

      pt.second.sx = smoothing * old_pt.sx + (1 - smoothing) * pt.second.x;
      pt.second.sy = smoothing * old_pt.sy + (1 - smoothing) * pt.second.y;

    } else {

      // This is an old point, so check what is happening to it
      check_drag(history[pt.first], pt.second);
      check_hold(history[pt.first], pt.second);
      check_click(history[pt.first], pt.second);

      if (pt.second.state & State::TOUCH_DOWN)
        pt.second.state &= ~State::TOUCH_DOWN;

      assert(previous_state.find(pt.first) != previous_state.end());
      TouchPoint old_pt = previous_state.find(pt.first)->second;

      pt.second.sx = smoothing * old_pt.sx + (1 - smoothing) * pt.second.x;
      pt.second.sy = smoothing * old_pt.sy + (1 - smoothing) * pt.second.y;
    }
  }

  velocityEstimator.cleanup();

  state = 0;
}

void TouchState::clearReleasedStates() {
  // Copy associations and history
  std::map<TouchPointId, void*> new_association;
  std::map<TouchPointId, HistoryState> new_history;
  std::map<TouchPointId, TouchPoint> new_current;

  for (auto &pt : current_state) {
    if (pt.second.state & State::RELEASE) {
      history[pt.first].point.state |= State::RELEASE;
      continue;
    }

    new_current[pt.first] = pt.second;
  }
  
  for (auto &pt : history) {
    if (pt.second.point.state & State::RELEASE &&
        clock::now() - pt.second.time > multi_time) continue;
    
    if (current_association.find(pt.first) != current_association.end())
      new_association[pt.first] = current_association[pt.first];
    new_history[pt.first] = history[pt.first];
  }
  
  current_association.swap(new_association);
  history.swap(new_history);
  current_state.swap(new_current);
}

TouchState::TouchPoint::TouchPoint()
  : x(0), y(0), vx(0), vy(0), sx(0), sy(0), id(0), state(0), clicks(1) {}

TouchState::TouchPoint::TouchPoint(TouchPointId id, float x, float y)
  : x(x), y(y), vx(0), vy(0), sx(x), sy(y), id(id), state(0), clicks(1) {}


void TouchState::check_multi(TouchPoint &new_pt) {

  std::map<TouchPointId, void*> new_association;
  std::map<TouchPointId, HistoryState> new_history;

  for (auto &pt : history) {
    auto &point = pt.second.point;

    // Not a release, therefore an active finger, not multi
    if (! (point.state & State::RELEASE)) {
      if (current_association.find(pt.first) != current_association.end())
        new_association[pt.first] = current_association[pt.first];
      new_history[pt.first] = history[pt.first];
      continue;
    }

    // Too far away, therefore not multi, but might become in the future
    if ((point.x - new_pt.x) * (point.x - new_pt.x) +
        (point.y - new_pt.y) * (point.y - new_pt.y)
        > move_magnitude * move_magnitude) {
      if (current_association.find(pt.first) != current_association.end())
        new_association[pt.first] = current_association[pt.first];
      new_history[pt.first] = history[pt.first];
      continue;
    }

    // Copy old association to this new (multi) touch point
    if (current_association.find(pt.first) != current_association.end())
      new_association[new_pt.id] = current_association[pt.first];

    new_pt.clicks = point.clicks + 1;
    new_pt.state |= State::MULTI;

    break;
  }

  current_association.swap(new_association);
  history.swap(new_history);
}

void TouchState::check_drag(HistoryState hist, TouchPoint &new_pt) {
  assert(hist.point.id == new_pt.id);
  if (new_pt.state & State::DRAG) return;
  if ((hist.point.x - new_pt.x) * (hist.point.x - new_pt.x) +
      (hist.point.y - new_pt.y) * (hist.point.y - new_pt.y)
      < move_magnitude * move_magnitude)
    return;
  
  new_pt.state |= State::DRAG;
}

void TouchState::check_hold(HistoryState hist, TouchPoint &new_pt) {
  assert(hist.point.id == new_pt.id);
  if (new_pt.state & State::DRAG) return;
  if (new_pt.state & State::HOLD) return;
  if ((clock::now() - hist.time) < hold_time)
    return;
  
  new_pt.state |= State::HOLD;
}

void TouchState::check_click(HistoryState hist, TouchPoint &new_pt) {
  assert(hist.point.id == new_pt.id);
  if (new_pt.state & State::CLICK) return;
  if (new_pt.state & State::DRAG) return;
  if (! (new_pt.state & State::RELEASE)) return;
  if ((clock::now() - hist.time) > click_time)
    return;
  
  new_pt.state |= State::CLICK;
}

END_NAMESPACE_GMTOUCH;
