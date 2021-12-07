
#ifndef GRAMODS_TOUCH_TOUCHSTATE
#define GRAMODS_TOUCH_TOUCHSTATE

#include <gmTouch/config.hh>

#include <gmMisc/EFFOAW.hh>

#include <Eigen/Eigen>

#include <vector>
#include <chrono>
#include <map>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>

#include <assert.h>

BEGIN_NAMESPACE_GMTOUCH;

/**
 * The TouchState class represent the internal states during touch
 * interaction. It supports association of touch points with
 * objects, "multi click" (double click and more), and provides
 * states to differentiate between dragging and held touch
 * points. For example, a user may tripple click, hold and then
 * drag, which is then reflected by the internal states of the touch
 * point. For example, an object may choose to associate a touch
 * point to itself only if it represents a tripple
 * click-and-hold-then-drag.
 *
 * Typical usage:
 * \code{.cpp}
 * // Copy event data into TouchState object (in SDL window handler) ---
 * touchState.eventsInit(width, height);
 * touchState.setCurrentProjection(camera);
 *
 * SDL_Event event;
 * while(SDL_PollEvent(&event)) {
 *   touchState.getEventAdaptor<gmTouch::SDLEventAdaptor>().handleEvent(event);
 * }
 * touchState.eventsDone();
 *
 * ...
 *
 * // Associate touch points with this object (in interaction object class) ---
 * TouchPoints touchPoints;
 * if (touchState.getTouchPoints(this, touchPoints) < REQUIRED_POINTS) {
 *   TouchPoints candidates;
 *   touchState.getTouchPoints(nullptr, candidates);
 *   for (auto& pt : candidates)
 *     if (isInside(pt.x, pt.y)) // Checks if the point hits this object
 *       touchState.setAssociation(pt.id, this);
 * }
 *
 * TouchPoints currentTouchPoints, previousTouchPoints;
 * if (touchState.getTouchPoints(this, currentTouchPoints, previousTouchPoints) < REQUIRED_POINTS)
 *   return;
 *
 * // Use touch points here
 * \endcode
 */
class TouchState {

public:

  /// Creates an empty TouchState instance.
  TouchState();

  /// Destroys the TouchState instance.
  ~TouchState();

  /// The clock used in this class.
  typedef std::chrono::steady_clock clock;

  /**
   * The internal type of a touch point id. Observe that the
   * underlying event system may use a different type and wrap
   * around at any value.
   */
  typedef int64_t TouchPointId;

  /**
   * The touch point id used for mouse pointer simulating touch.
   */
  static const TouchPointId MOUSE_STATE_ID;

  /**
   * Touch states that can be bitwise combined (except NONE). Check
   * with bitwise AND, e g <tt>if (tp.state & State::DRAG)</tt>.
   */
  struct State {
    static const unsigned char NONE = 0;

    /// Touch point has previosly been moved while down.
    static const unsigned char DRAG = 0x01 << 0;

    /// Touch point was held still while first pressed.
    static const unsigned char HOLD = 0x01 << 1;

    /**
     * Touch point is a "double" click, or more.
     * \see TouchState::TouchPoint::clicks
     */
    static const unsigned char MULTI = 0x01 << 2;

    /// Touch point is newly down this frame. A multi click touch
    /// point will also be considered newly down.
    static const unsigned char TOUCH_DOWN = 0x01 << 3;

    /// Touch point was released.
    static const unsigned char RELEASE = 0x01 << 4;

    /// Touch point was a short click - pressed and depressed at the
    /// same spot within a short period of time.
    static const unsigned char CLICK = 0x01 << 5;
  };

  /**
   * A single (potentially smoothed) touch point.
   */
  struct TouchPoint {

    /// Constructs a positioned touch point with id.
    TouchPoint(TouchPointId id, float x, float y);

    /// Constructs an empty touch point.
    TouchPoint();

    /// x position in (sub) pixels from left edge.
    float x;

    /// y position in (sub) pixels from top edge.
    float y;

    /// x velocity in pixels per second from left edge.
    float vx;

    /// y velocity in pixels per second from top edge.
    float vy;

    /// x position estimated by the recent motion.
    float ex;

    /// y position estimated by the recent motion.
    float ey;

    /// smoothed version of the x position.
    float sx;

    /// smoothed version of the y position.
    float sy;

    /// internal id of the touch point.
    TouchPointId id;

    /**
     * State of the touch point expressed as a bitwise combination.
     *
     * \see State
     */
    unsigned char state;

    /**
     * The number of times this position was clicked in succession,
     * for example two (2) for a double click.
     */
    unsigned int clicks;
  };

  /**
   * List of touch points.
   */
  typedef std::vector<TouchPoint> TouchPoints;

  /**
   * A 3D line representation of a touch point.
   */
  struct TouchLine {

    /// The origin of the 3D line.
    Eigen::Vector3f x;

    /// The direction of the 3D line.
    Eigen::Vector3f v;

    /// internal id of the touch point.
    TouchPointId id;

    /**
     * State of the touch point expressed as a bitwise combination.
     *
     * \see State
     */
    unsigned char state;

    /**
     * The number of times this position was clicked in succession,
     * for example two (2) for a double click.
     */
    unsigned int clicks;
  };

  /**
   * List of touch lines.
   */
  typedef std::vector<TouchLine> TouchLines;

  /** @name Get touch points
   * Methods used to extract current touch points and the their states.
   */
  ///@{

  /**
   * Checks whether the TouchState is empty of touch points.
   */
  bool empty() const { return current_state.empty(); }

  /**
   * Gets the current touch points, and returns the point count.
   *
   * @param[out] current The resulting touch points.
   * @return The number of touch points extracted.
   */
  int getTouchPoints(TouchPoints &current) const;

  /**
   * Gets the current touch points and their corresponding previous
   * positions, and returns the point count. This will only provide
   * touch points with both current and previous states available.
   *
   * @param[out] current The most current touch points.
   * @param[out] previous The touch points from the previous time frame.
   * @return The number of touch points extracted.
   */
  int getTouchPoints(TouchPoints &current, TouchPoints &previous) const;

  /**
   * Gets the current touch points associated to the specified
   * pointer, and returns the point count. Unassociated touch points
   * can be extraced with nullptr as first argument.
   *
   * @param[in] ass The association to extract touch points for, or
   * nullptr for unassociated points.
   *
   * @param[out] current The resulting touch points.
   *
   * @return The number of touch points extracted.
   */
  int getTouchPoints(void *ass, TouchPoints &current) const;

  /**
   * Gets the current touch points associated to the specified
   * pointer, and their corresponding previous positions, and
   * returns the point count. This will only provide touch points
   * with both current and previous states available. Unassociated
   * touch points can be extraced with nullptr as first argument.
   *
   * @param[in] ass The association to extract touch points for, or
   * nullptr for unassociated points.
   *
   * @param[out] current The resulting touch points.
   *
   * @param[out] previous The touch points from the previous time frame.
   *
   * @return The number of touch points extracted.
   */
  int getTouchPoints(void *ass, TouchPoints &current, TouchPoints &previous) const;

  /**
   * Gets the current touch points as an association
   * map. Unassociated touch points will be associated with nullptr.
   *
   * @param[out] current The association map between objects (or
   * nullptr) and their associated touch points.
   */
  int getTouchPoints(std::map<void*, TouchPoints> &current) const;

  /**
   * Gets the current touch points as an association
   * map. Unassociated touch points will be associated with nullptr.
   *
   * @param[out] current The association map between objects (or
   * nullptr) and their associated touch points.
   * @param[out] previous The association map between objects (or
   * nullptr) and their associated previous touch points.
   */
  int getTouchPoints(std::map<void*, TouchPoints> &current,
                     std::map<void*, TouchPoints> &previous) const;

  /**
   * Associates an unassociated touch point id with an object.
   *
   * @param[in] id The touch point id that should be associated.
   * @param[in] pt A void pointer to the object to associate with.
   * @return True iff an association was successfully created.
   */
  bool setAssociation(TouchPointId id, void* pt);

  /**
   * Removes the association of the specified touch point id with
   * the specified object.
   *
   * @param[in] id The touch point id that should be unassociated.
   * @param[in] pt A void pointer to the currently associated object.
   * @return True iff an association was successfully removed.
   */
  bool unsetAssociation(TouchPointId id, void* pt);

  /**
   * Gets the touch point id associated with a specified object.
   *
   * @param[in] id The id of the touch point that is associated.
   *
   * @param[out] pt A pointer to a void pointer, set to the pointer
   * that is associated with the provided id, or nullptr if only
   * found/not found is of interest.
   *
   * @return True iff an association was found.
   */
  bool getAssociation(TouchPointId id, void** pt) const;

  ///!@}

  /** @name 3D projection methods
   * Methods used handle touch states' 3D behaviour.
   */
  ///@{

  /**
   * Sets the current (inverted) projection matrix to use when
   * back-projecting the 2D touch points into 3D. This can be
   * calculated by inverting the matrix product of 1) viewport
   * window matrix, 2) projection matrix and 3) view matrix
   * (W*P*V)^-1, when using column vector matrices.
   *
   * It is more convenient to use a TouchState::CameraAdaptor.
   *
   * If the view changes changing over time, one of these methods
   * needs to be called every frame, between calling eventsInit and
   * eventsDone and before extrating 3D lines. If the view is
   * static, the method needs to be called at least twice, to make
   * the current matrix be copied to previous state.
   *
   * \see TouchState::getTouchLines
   */
  void setCurrentProjection(Eigen::Matrix4f WPV_inv);

  /**
   * Base type for camera adaptors providing means to input camera data
   * from different scenegraph or graphics platforms.
   *
   * Get the platform/library dependent adaptor by calling
   * TouchState::getCameraAdaptor.
   */
  struct CameraAdaptor {

    virtual ~CameraAdaptor() {}

    /**
     * Called by the owner (TouchState) when eventsInit is called.
     */
    virtual void init(int width, int height);

    /**
     * Called by the owner (TouchState) when eventsDone is called.
     */
    virtual void done();

  protected:

    /**
     * Sets the current (inverted) projection matrix to use when
     * back-projecting the 2D touch points into 3D. This can be
     * calculated by inverting the matrix product of 1) viewport
     * window matrix, 2) projection matrix and 3) view matrix
     * (W*P*V)^-1, when using column vector matrices.
     *
     * If the view changes changing over time, one of these methods
     * needs to be called every frame, between calling eventsInit and
     * eventsDone and before extrating 3D lines. If the view is
     * static, the method needs to be called at least twice, to make
     * the current matrix be copied to previous state.
     *
     * \see TouchState::getTouchLines
     */
    void setCurrentProjection(Eigen::Matrix4f WPV_inv);

  private:

    TouchState *owner;
    friend TouchState;
  };
  friend CameraAdaptor;

  /**
   * Returns a reference to the internal camera adaptor for the
   * specified type. The adaptor is instantiated upon the first call
   * to this method and deleted when the TouchState is destroyed.
   */
  template<class T>
  T & getCameraAdaptor();

  /**
   * Gets the current touch lines and returns true, if lines can be
   * extracted from the current touch points. False is returned
   * otherwise.
   *
   * @param[out] current The resulting 3D lines, originating at the near plane.
   * @return True if current lines could be extracted.
   */
  bool getTouchLines(TouchLines &current) const;

  /**
   * Gets the current and previous touch lines and returns true, if
   * lines can be extracted from the current and previous touch
   * points. False is returned otherwise.
   *
   * @param[out] current The current 3D lines, originating at the near plane.
   * @param[out] previous The previous 3D lines, originating at the near plane.
   * @return True if both current and previous lines could be extracted.
   */
  bool getTouchLines(TouchLines &current, TouchLines &previous) const;

  /**
   * Gets the current touch lines and returns true, if lines can be
   * extracted from the current touch points. False is returned
   * otherwise.
   *
   * @param[in] ass
   * @param[out] current The resulting 3D lines, originating at the near plane.
   * @return True if current lines could be extracted.
   */
  bool getTouchLines(void *ass, TouchLines &current) const;

  /**
   * Gets the current and previous touch lines and returns true, if
   * lines can be extracted from the current and previous touch
   * points. False is returned otherwise.
   *
   * @param[in] ass
   * @param[out] current The current 3D lines, originating at the near plane.
   * @param[out] previous The previous 3D lines, originating at the near plane.
   * @return True if both current and previous lines could be extracted.
   */
  bool getTouchLines(void *ass, TouchLines &current, TouchLines &previous) const;

  /**
   * Gets the current touch lines as an association
   * map. Unassociated touch lines will be associated with nullptr.
   *
   * @param[out] current The association map between objects (or
   * nullptr) and their associated touch lines.
   * @return True if both current lines could be extracted.
   */
  bool getTouchLines(std::map<void*, TouchLines> &current) const;

  /**
   * Gets the current touch lines as an association
   * map. Unassociated touch lines will be associated with nullptr.
   *
   * @param[out] current The association map between objects (or
   * nullptr) and their associated touch lines.
   * @param[out] previous The association map between objects (or
   * nullptr) and their associated previous touch lines.
   * @return True if both current and previous lines could be extracted.
   */
  bool getTouchLines(std::map<void*, TouchLines> &current,
                     std::map<void*, TouchLines> &previous) const;

  ///!@}

  /** @name Mouse simulation methods
   * Methods used to allow interaction with mouse, when touch is not available.
   */
  ///@{

  /**
   * Returns the mouse wheel value, positive up and negative down.
   */
  float getMouseWheel() const;

  /**
   * Returns true if the mouse button is down, false otherwise.
   */
  bool getMouseDown() const;

  /**
   * Get the point point.
   */
  void getMousePoint(int &x, int &y) const;

  /**
   * Get the point point as a touch line. Returns true if the line
   * could be calculated.
   */
  bool getMouseLine(Eigen::Vector3f &x,
                    Eigen::Vector3f &v) const;

  ///!@}

  /** @name Parameters
   * Methods used to adjust the general behaviour of the class.
   */
  ///@{

  /**
   * Sets the level of smoothing to use, to reduce jerky actions
   * caused by noisy and jittery input devices. The value must be
   * between zero (inclusive, representing no smoothing, which is
   * the default) and one (exclusive, representing no motion).
   */
  void setSmoothing(float r);

  /**
   * Gets the level of smoothing currently in use.
   * \see setSmoothing
   */
  float getSmoothing();

  /**
   * Incoming mouse events are used to simulate touch, but if this
   * is set to true (default) this behaviour is deactivated upon
   * incoming touch events.
   */
  void setRemoveMouseUponTouch(bool on);

  /**
   * Gets the mouse filtering state.
   * \see setRemoveMouseUponTouch
   */
  bool getRemoveMouseUponTouch();

  /**
   * Sets the magnitude of movement (Euclidean distance in pixels,
   * default 10) that is allowed before the touch point is
   * considered to be moved.
   *
   * This affects when the states State::DRAG and State::MULTI are
   * applied. Also, velocity estimation uses this value as a measure
   * of touch noise.
   *
   * @param[in] dist Euclidean distance in pixels
   */
  void setMoveMagnitude(float dist);

  /**
   * Gets the movement magnitude that is considered a touch movement.
   * \see setMoveMagnitude
   */
  float getMoveMagnitude();

  /**
   * Sets the time that a touch point must be held without being
   * dragged before its state get the HOLD flag, default 2 seconds.
   */
  void setHoldTime(clock::duration time);

  /**
   * Gets the hold time.
   * \see setHoldTime
   */
  clock::duration getHoldTime();

  /**
   * Sets the time that a touch point can be held before release to
   * get a CLICK flag, default 500 ms.
   */
  void setClickTime(clock::duration time);

  /**
   * Gets the click time.
   * \see setClickTime
   */
  clock::duration getClickTime();

  /**
   * Sets the maximum amount of time (default 500 ms) between two
   * touches at the same point that is considered a multi click of
   * the same touch point.
   */
  void setMultiClickTime(clock::duration time);

  /**
   * Gets the multi click time.
   * \see setMultiClickTime
   */
  clock::duration getMultiClickTime();

  ///!@}

  /** @name Updating internal states
   * Methods used to fill and update the set of states.
   */
  ///@{

  /**
   * Base type for event adaptors providing means to input events
   * from different platforms and window libraries.
   *
   * Get the platform/library dependent adaptor by calling
   * getEventAdaptor.
   */
  struct EventAdaptor {

    virtual ~EventAdaptor() {}

    /**
     * Called by the owner (TouchState) when eventsInit is called.
     */
    virtual void init(int, int);

    /**
     * Called by the owner (TouchState) when eventsDone is called.
     */
    virtual void done();

  protected:

    /**
     * Adds a touch point sample
     *
     * @param id The id of the touch point
     * @param x position in (sub) pixels from left edge.
     * @param y position in (sub) pixels from top edge.
     */
    void addTouchState(TouchPointId id, float x, float y);

    /**
     * Removes the touch point with the specified id.
     */
    void removeTouchState(TouchPointId id, float x, float y);

    /**
     * Add the specified mouse state. This will also add and remove
     * simulated touch states.
     */
    void addMouseState(float x, float y, bool mouse_down);

    /**
     * Sets the wheel value, positive up and negative down.
     */
    void addMouseWheel(float s);

  private:

    TouchState *owner;
    friend TouchState;
  };
  friend EventAdaptor;

  /**
   * Returns a reference to the internal event adaptor for the
   * specified type. The adaptor is instantiated upon the first call
   * to this method and deleted when the TouchState is destroyed.
   */
  template<class T>
  T & getEventAdaptor();

  /**
   * Initializes the event handling. Call this before calling
   * handleEvents.
   *
   * @param[in] width The current view width in pixels
   * @param[in] height The current view height in pixels
   */
  void eventsInit(int width, int height);

  /**
   * Flags the internal touch states as complete. Call this after
   * calling handleEvent with all currently available events.
   */
  void eventsDone();

  ///!@}

private:

  struct HistoryState {
    TouchPoint point;
    clock::time_point time;
  };

  gmMisc::EFFOAW velocityEstimator;

  /**
   * Adds a touch point sample
   * @param id The id of the touch point
   * @param x position in (sub) pixels from left edge.
   * @param y position in (sub) pixels from top edge.
   * @param time the time of the event in seconds.
   */
  void addTouchState(TouchPointId id, float x, float y);

  /**
   * Removes the touch point with the specified id.
   */
  void removeTouchState(TouchPointId id, float x, float y);

  /**
   * Add the specified mouse state. This will also add and remove
   * simulated touch states.
   */
  void addMouseState(float x, float y, bool mouse_down);

  /**
   * Sets the wheel value, positive up and negative down.
   */
  void addMouseWheel(float s);

  /**
   * Internal
   */
  void addState(TouchPointId id, float x, float y);

  clock::time_point start_time = clock::time_point::min();
  float noise_level;
  float velocity_reestimation_rate;

  float smoothing;
  float move_magnitude;
  clock::duration hold_time;
  clock::duration click_time;
  clock::duration multi_time;

  bool remove_mouse_upon_touch = true;
  bool use_mouse = true;

  std::map<TouchPointId, TouchPoint> current_state;
  std::map<TouchPointId, TouchPoint> previous_state;

  std::map<TouchPointId, void*> last_association;
  std::map<TouchPointId, void*> current_association;

  std::map<TouchPointId, HistoryState> history;

  /**
   * Checks if this new touch point is really a multi click, based
   * on historical information.
   *
   * @return True if the point is actually multi click, false
   * otherwise.
   */
  void check_multi(TouchPoint &new_pt);

  /**
   * Checks if the hist/new_pt pair represents a drag motion, and
   * sets the corresponding flag in new_pt.
   *
   * @param[in] hist The history data of the touch point to check
   * @param[in,out] The touch point to check and update
   */
  void check_drag(HistoryState hist, TouchPoint &new_pt);

  /**
   * Checks if the hist/new_pt pair represents a hold action, and
   * sets the corresponding flag in new_pt.
   *
   * @param[in] hist The history data of the touch point to check
   * @param[in,out] The touch point to check and update
   */
  void check_hold(HistoryState hist, TouchPoint &new_pt);

  /**
   * Checks if the hist/new_pt pair represents a click action, and
   * sets the corresponding flag in new_pt.
   *
   * @param[in] hist The history data of the touch point to check
   * @param[in,out] The touch point to check and update
   */
  void check_click(HistoryState hist, TouchPoint &new_pt);

  /**
   * Removes states that were previously released (has state
   * RELEASE). This is called from eventsInit().
   */
  void clearReleasedStates();

  int state = 0;

  /**
   * Converts the specified touch point into a touch line using the
   * specified inverse projection matrix.
   */
  TouchLine touchPointToTouchLine(TouchPoint pt, Eigen::Matrix4f WPV_inv) const;

  Eigen::Matrix4f current_WPV_inv;
  bool current_WPV_inv_valid = false;

  Eigen::Matrix4f previous_WPV_inv;
  bool previous_WPV_inv_valid = false;

  int current_height = -1;
  int previous_height = -1;
  int current_width = -1;
  int previous_width = -1;

  float mouse_wheel = 0.f;
  bool mouse_down = false;
  int mouse_point_x = 0;
  int mouse_point_y = 0;

  std::unordered_map<std::type_index, EventAdaptor*> event_adaptors;
  std::unordered_map<std::type_index, CameraAdaptor*> camera_adaptors;
};


template<class T>
T & TouchState::getEventAdaptor() {
  if (event_adaptors.find(typeid(T)) == event_adaptors.end()) {
    event_adaptors[typeid(T)] = new T;
    event_adaptors[typeid(T)]->owner = this;
  }
  assert(dynamic_cast<T*>(event_adaptors[typeid(T)]));
  return *dynamic_cast<T*>(event_adaptors[typeid(T)]);
}

template<class T>
T & TouchState::getCameraAdaptor() {
  if (camera_adaptors.find(typeid(T)) == camera_adaptors.end()) {
    camera_adaptors[typeid(T)] = new T;
    camera_adaptors[typeid(T)]->owner = this;
  }
  assert(dynamic_cast<T*>(camera_adaptors[typeid(T)]));
  return *dynamic_cast<T*>(camera_adaptors[typeid(T)]);
}

END_NAMESPACE_GMTOUCH;

#endif
