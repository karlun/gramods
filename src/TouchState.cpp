
#include <touchlib/TouchState.hh>

#include <stdexcept>
#include <limits>
#include <assert.h>

using namespace touchlib;

#define DEFAULT_DRAG_MAGNITUDE 10
#define DEFAULT_DRAG_HISTORY_LENGTH 10
#define DEFAULT_DRAG_HISTORY_DURATION 2.0
#define DEFAULT_DRAG_ERROR 5.0

TouchState::TouchState()
  : state(0),
    use_mouse(true),
    mouse_down(false),
    remove_mouse_upon_touch(true),
    smoothing(0.f),
    drag_magnitude(DEFAULT_DRAG_MAGNITUDE),
    hold_time(std::chrono::seconds(2)),
    current_WPV_inv_valid(false), 
    previous_WPV_inv_valid(false) {
  
  velocityEstimator.setHistoryLength(DEFAULT_DRAG_HISTORY_LENGTH);
  velocityEstimator.setHistoryDuration(DEFAULT_DRAG_HISTORY_DURATION);
}

int TouchState::getTouchPoints(TouchPoints &current) const {
  assert(state == 0);
  
  current.clear();
  current.reserve(current_state.size());
  
  for (auto tp : current_state)
    current.push_back(tp.second);
  
  return current.size();
}

int TouchState::getTouchPoints(TouchPoints &current, TouchPoints &previous) const {
  assert(state == 0);
  
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

int TouchState::getTouchPoints(std::map<void*, TouchPoints> &current) const {
  assert(state == 0);

  current.clear();

  std::map<TouchPointId, TouchPoint>::const_iterator tp_it = current_state.begin();
  std::map<TouchPointId, void*>::const_iterator ass_it = association.begin();

  while (true)
    if (tp_it->first < ass_it->first) {
      current[nullptr].push_back(tp_it->second);
      if (++tp_it == current_state.end())
        break;
    } else if (tp_it->first > ass_it->first) {
      assert(0);
      if (++ass_it == association.end())
        break;
    } else {
      current[ass_it->second].push_back(tp_it->second);
      if (++tp_it == current_state.end() ||
          ++ass_it == association.end())
        break;
    }
  for (; tp_it != current_state.end(); ++tp_it)
    current[nullptr].push_back(tp_it->second);
  
  return current.size();
}

int TouchState::getTouchPoints(std::map<void*, TouchPoints> &current,
                               std::map<void*, TouchPoints> &previous) const {
  assert(state == 0);
  
  current.clear();
  previous.clear();

  std::map<TouchPointId, TouchPoint>::const_iterator tp_it = current_state.begin();
  std::map<TouchPointId, void*>::const_iterator ass_it = association.begin();

  while (tp_it != current_state.end() && ass_it != association.end()) {
    if (tp_it->first < ass_it->first) {
      
      if (previous_state.find(tp_it->second.id) != previous_state.end()) {
        current[nullptr].push_back(tp_it->second);
        previous[nullptr].push_back(previous_state.find(tp_it->second.id)->second);
      }
      
      if (++tp_it == current_state.end())
        break;
    } else if (tp_it->first > ass_it->first) {
      assert(0);
      if (++ass_it == association.end())
        break;
    } else {

      if (previous_state.find(tp_it->second.id) != previous_state.end()) {
        current[ass_it->second].push_back(tp_it->second);
        previous[ass_it->second].push_back(previous_state.find(tp_it->second.id)->second);
      }
      
      if (++tp_it == current_state.end() ||
          ++ass_it == association.end())
        break;
    }
  }
  for (; tp_it != current_state.end(); ++tp_it) {
    if (previous_state.find(tp_it->second.id) != previous_state.end()) {
      current[nullptr].push_back(tp_it->second);
      previous[nullptr].push_back(previous_state.find(tp_it->second.id)->second);
    }
  }
  
  assert(current.size() == previous.size());
  return current.size();
}

int TouchState::getUnassociatedTouchPoints(TouchPoints &current) const {
  assert(state == 0);
  
  current.clear();
  current.reserve(current_state.size() - association.size());

  std::map<TouchPointId, TouchPoint>::const_iterator tp_it = current_state.begin();
  std::map<TouchPointId, void*>::const_iterator ass_it = association.begin();

  while (true)
    if (tp_it->first < ass_it->first) {
      current.push_back(tp_it->second);
      if (++tp_it == current_state.end())
        break;
    } else if (tp_it->first > ass_it->first) {
      assert(0);
      if (++ass_it == association.end())
        break;
    } else {
      if (++tp_it == current_state.end() ||
          ++ass_it == association.end())
        break;
    }
  for (; tp_it != current_state.end(); ++tp_it)
    current.push_back(tp_it->second);
  
  return current.size();
}


bool TouchState::setAssociation(TouchPointId id, void* pt) {
  assert(state == 0);
  if (pt == nullptr) throw std::invalid_argument("pt == nullptr");
  if (current_state.count(id) == 0) return false;
  association[id] = pt;
  return true;
}

bool TouchState::getAssociation(TouchPointId id, void* pt) const {
  assert(state == 0);
  if (association.find(id) == association.end()) return false;
  pt = association.find(id)->second;
  return true;
}

bool TouchState::getAssociation(void* pt, TouchPointId &id) const {
  assert(state == 0);
  for (auto ass : association)
    if (ass.second == pt) {
      id = ass.first;
      return true;
    }
  return false;
}


#ifdef TOUCHLIB_ENABLE_OpenSceneGraph
bool TouchState::setCurrentProjection(osg::Camera * camera) {
  assert(state == 1);
  osg::Matrix VPW = (camera->getViewMatrix()
                     * camera->getProjectionMatrix()
                     * camera->getViewport()->computeWindowMatrix());
  osg::Matrix invVPW;
  if (!invVPW.invert(VPW))
    return false;

  current_WPV_inv = utm50_utils::Matrix4f::fromArrayCM(invVPW.ptr());
  current_WPV_inv_valid = true;

  return true;
}
#endif

void TouchState::setCurrentProjection(utm50_utils::Matrix4f WPV_inv) {
  assert(state == 1);
  current_WPV_inv = WPV_inv;
  current_WPV_inv_valid = true;
}

bool TouchState::getTouchLines(TouchLines &current) const {
  assert(state == 0);
  if (!current_WPV_inv_valid) return false;
  
  current.clear();
  
  TouchPoints cur_pts;
  getTouchPoints(cur_pts);
  current.reserve(cur_pts.size());

  for (auto pt : cur_pts) {
    
    utm50_utils::Vector4f nearPoint =
      (current_WPV_inv * utm50_utils::Vector4f(pt.sx, current_height - pt.sy, 0.f)).renormalized();
    utm50_utils::Vector4f farPoint =
      (current_WPV_inv * utm50_utils::Vector4f(pt.sx, current_height - pt.sy, 1.f)).renormalized();

    TouchLine tl = { nearPoint,
                     utm50_utils::Vector3f(farPoint - nearPoint).normalized(),
                     pt.id,
                     pt.state };
    current.push_back(tl);
  }

  return true;
}

bool TouchState::getTouchLines(TouchLines &current, TouchLines &previous) const {
  assert(state == 0);
  if (!current_WPV_inv_valid) return false;
  if (!previous_WPV_inv_valid) return false;

  current.clear();
  previous.clear();
  
  TouchPoints cur_pts, pre_pts;
  getTouchPoints(cur_pts, pre_pts);

  current.reserve(cur_pts.size());
  for (auto pt : cur_pts) {
    
    utm50_utils::Vector4f nearPoint =
      (current_WPV_inv * utm50_utils::Vector4f(pt.sx, current_height - pt.sy, 0.f)).renormalized();
    utm50_utils::Vector4f farPoint =
      (current_WPV_inv * utm50_utils::Vector4f(pt.sx, current_height - pt.sy, 1.f)).renormalized();
    
    TouchLine tl = { nearPoint,
                     utm50_utils::Vector3f(farPoint - nearPoint).normalized(),
                     pt.id,
                     pt.state };
    current.push_back(tl);
  }

  previous.reserve(pre_pts.size());
  for (auto pt : pre_pts) {
    
    utm50_utils::Vector4f nearPoint =
      (previous_WPV_inv * utm50_utils::Vector4f(pt.sx, previous_height - pt.sy, 0.f)).renormalized();
    utm50_utils::Vector4f farPoint =
      (previous_WPV_inv * utm50_utils::Vector4f(pt.sx, previous_height - pt.sy, 1.f)).renormalized();
     
    TouchLine tl = { nearPoint,
                     utm50_utils::Vector3f(farPoint - nearPoint).normalized(),
                     pt.id,
                     pt.state };
    previous.push_back(tl);
  }
  
  return true;
}

bool TouchState::getTouchLines(std::map<void*, TouchLines> &current) const {
  assert(state == 0);
  if (!current_WPV_inv_valid) return false;

  current.clear();
  
  std::map<void*, TouchPoints> cur_pts;
  getTouchPoints(cur_pts);
  
  for (auto it : cur_pts) {
    TouchLines lines;
    lines.reserve(it.second.size());
    
    for (auto pt : it.second) {
      
      utm50_utils::Vector4f nearPoint =
        (current_WPV_inv * utm50_utils::Vector4f(pt.sx, current_height - pt.sy, 0.f)).renormalized();
      utm50_utils::Vector4f farPoint =
        (current_WPV_inv * utm50_utils::Vector4f(pt.sx, current_height - pt.sy, 1.f)).renormalized();
      
      TouchLine tl = { nearPoint,
                       utm50_utils::Vector3f(farPoint - nearPoint).normalized(),
                       pt.id,
                       pt.state };
      lines.push_back(tl);
    }

    current[it.first].swap(lines);
  }
  
  return true;
}

bool TouchState::getTouchLines(std::map<void*, TouchLines> &current,
                               std::map<void*, TouchLines> &previous) const {
  assert(state == 0);
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
    
    for (auto pt : it.second) {

      utm50_utils::Vector4f nearPoint =
        (current_WPV_inv * utm50_utils::Vector4f(pt.sx, current_height - pt.sy, 0.f)).renormalized();
      utm50_utils::Vector4f farPoint =
        (current_WPV_inv * utm50_utils::Vector4f(pt.sx, current_height - pt.sy, 1.f)).renormalized();

      TouchLine tl = { nearPoint,
                       utm50_utils::Vector3f(farPoint - nearPoint).normalized(),
                       pt.id,
                       pt.state };
      lines.push_back(tl);
    }

    current[it.first].swap(lines);
  }

  for (auto it : pre_pts) {
    TouchLines lines;
    lines.reserve(it.second.size());
    
    for (auto pt : it.second) {
      
      utm50_utils::Vector4f nearPoint =
        (previous_WPV_inv * utm50_utils::Vector4f(pt.sx, previous_height - pt.sy, 0.f)).renormalized();
      utm50_utils::Vector4f farPoint =
        (previous_WPV_inv * utm50_utils::Vector4f(pt.sx, previous_height - pt.sy, 1.f)).renormalized();
      
      TouchLine tl = { nearPoint,
                       utm50_utils::Vector3f(farPoint - nearPoint).normalized(),
                       pt.id,
                       pt.state };
      lines.push_back(tl);
    }

    previous[it.first].swap(lines);
  }

  return true;
}


float TouchState::getMouseScroll() const {
  return mouse_wheel;
}

bool TouchState::getMouseDown() const {
  return mouse_down;
}

void TouchState::getMousePoint(int &x, int &y) const {
  x = mouse_point_x;
  y = mouse_point_y;
}

bool TouchState::getMouseLine(utm50_utils::Vector4f &x,
                              utm50_utils::Vector3f &v) const {
  if (! current_WPV_inv_valid) return false;

  int px, py;
  getMousePoint(px, py);
  
  utm50_utils::Vector4f nearPoint =
    (current_WPV_inv * utm50_utils::Vector4f(px, current_height - py, 0.f)).renormalized();
  utm50_utils::Vector4f farPoint =
    (current_WPV_inv * utm50_utils::Vector4f(px, current_height - py, 1.f)).renormalized();
  
  x = nearPoint;
  v = utm50_utils::Vector3f(farPoint - nearPoint).normalized();
  
  return true;
}


void TouchState::setSmoothing(float r) {
  if (r < 0.f) throw std::invalid_argument("r < 0");
  if (r >= 1.f) throw std::invalid_argument("r >= 1");
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

void TouchState::setDragMagnitude(float dist) {
  drag_magnitude = dist;
}

float TouchState::getDragMagnitude() {
  return drag_magnitude;
}

void TouchState::eventsInit(int width, int height) {
  assert(state == 0);
  state = 1;

  previous_state = current_state;
  
  previous_width = current_width;
  previous_height = current_height;

  previous_WPV_inv = current_WPV_inv;
  previous_WPV_inv_valid = current_WPV_inv_valid;
  
  current_width = width;
  current_height = height;

  mouse_wheel = 0.f;
}

#ifdef TOUCHLIB_ENABLE_SDL2
void TouchState::handleEvent(const SDL_Event& event) {
  assert(state == 1);
  
  switch (event.type) {
    
  case SDL_MOUSEMOTION:
    if (!mouse_down) return;
    
  case SDL_MOUSEBUTTONDOWN: {
    addState(event.motion.which, event.motion.x, event.motion.y,
             1e-3 * event.motion.timestamp, true);
    return;
  }
    
  case SDL_MOUSEBUTTONUP:
    removeState(event.motion.which, event.motion.x, event.motion.y, true);
    return;
    
  case SDL_MOUSEWHEEL:
    mouse_wheel = event.wheel.y;
    return;
    
  case SDL_FINGERDOWN: {
#ifdef __linux__
    float x = event.tfinger.x;
    float y = event.tfinger.y;
#else
    float x = event.tfinger.x * current_width;
    float y = event.tfinger.y * current_height;
#endif
    addState(event.tfinger.fingerId, x, y,
             1e-3 * event.tfinger.timestamp);
    return;
  }
    
  case SDL_FINGERMOTION: {
#ifdef __linux__
    float x = event.tfinger.x;
    float y = event.tfinger.y;
#else
    float x = event.tfinger.x * current_width;
    float y = event.tfinger.y * current_height;
#endif
    addState(event.tfinger.fingerId, x, y, 1e-3 * event.tfinger.timestamp);
    return;
  }
    
  case SDL_FINGERUP:
#ifdef __linux__
    float x = event.tfinger.x;
    float y = event.tfinger.y;
#else
    float x = event.tfinger.x * current_width;
    float y = event.tfinger.y * current_height;
#endif
    removeState(event.tfinger.fingerId, x, y);
    return;
  }
}
#endif

void TouchState::addState(TouchPointId id, float x, float y, double time, bool mouse) {
  assert(state == 1);

  if (mouse) {
    mouse_down = true;
    
    mouse_point_x = x;
    mouse_point_y = y;
    
    if (!use_mouse) return;
    
  } else {
    if (use_mouse && remove_mouse_upon_touch) use_mouse = false;
  }
  
  if (current_state.find(id) == current_state.end()) {
    TouchPoint p(id, 0, 0);
    current_state.insert(std::make_pair(id, p));
  }
  
  velocityEstimator.addSample(id, utm50_utils::Vector3f(x, y, 0), time);
}

void TouchState::removeState(TouchPointId id, float x, float y, bool mouse) {
  assert(state == 1);

  if (mouse) {
    mouse_down = false;
    
    mouse_point_x = x;
    mouse_point_y = y;
  }
  
  if (current_state.find(id) == current_state.end()) return;
  
  current_state[id].state |= State::RELEASE;
}

void TouchState::eventsDone() {
  assert(state == 1);
  
  // Copy associations and history
  std::map<TouchPointId, void*> new_association;
  std::map<TouchPointId, HistoryState> new_history;
  std::map<TouchPointId, TouchPoint> new_current;
  
  for (auto &pt : current_state) {
    
    double last_time = velocityEstimator.getLastSampleTime(pt.first);
    utm50_utils::Vector3f vel = velocityEstimator.estimateVelocity(pt.first, DEFAULT_DRAG_ERROR);
    utm50_utils::Vector3f pos = velocityEstimator.estimatePosition(pt.first, DEFAULT_DRAG_ERROR, last_time);

    pt.second.x = pos[0];
    pt.second.y = pos[1];
    pt.second.vx = vel[0];
    pt.second.vy = vel[1];
    
    if (history.find(pt.first) == history.end()) {
      // This is a new point, that has no history
      HistoryState hs = { pt.second, clock::now() };
      history[pt.first] = hs;
      
      pt.second.sx = pt.second.x;
      pt.second.sy = pt.second.y;
    } else {
      // This is an old point, so check what is happening to it
      check_drag(history[pt.first], pt.second);
      check_hold(history[pt.first], pt.second);
      
      assert(previous_state.find(pt.first) != previous_state.end());
      TouchPoint old_pt = previous_state.find(pt.first)->second;

      pt.second.sx = smoothing * old_pt.sx + (1 - smoothing) * pt.second.x;
      pt.second.sy = smoothing * old_pt.sy + (1 - smoothing) * pt.second.y;
    }
    
    if (! (pt.second.state & State::RELEASE)) {
      if (association.find(pt.first) != association.end())
        new_association[pt.first] = association[pt.first];
      new_history[pt.first] = history[pt.first];
      new_current[pt.first] = pt.second;
    }
  }

  association.swap(new_association);
  history.swap(new_history);
  current_state.swap(new_current);
  
  velocityEstimator.cleanup();
  
  state = 0;
}

TouchState::TouchPoint::TouchPoint()
  : x(0), y(0), vx(0), vy(0), sx(0), sy(0), id(0), state(0) {}

TouchState::TouchPoint::TouchPoint(TouchPointId id, float x, float y)
  : x(x), y(y), vx(0), vy(0), sx(x), sy(y), id(id), state(0) {}


void TouchState::check_drag(HistoryState hist, TouchPoint &new_pt) {
  assert(hist.point.id == new_pt.id);
  if (new_pt.state & State::DRAG) return;
  if ((hist.point.x - new_pt.x) * (hist.point.x - new_pt.x) +
      (hist.point.y - new_pt.y) * (hist.point.y - new_pt.y) < drag_magnitude)
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
