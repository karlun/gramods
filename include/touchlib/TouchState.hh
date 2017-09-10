
#ifndef __TOUCHLIB_TOUCHSTATE_HH__
#define __TOUCHLIB_TOUCHSTATE_HH__

#include <touchlib/config.hh>

#ifdef TOUCHLIB_ENABLE_OpenSceneGraph
#include <osg/Camera>
#endif

#include <touchlib/EFFOAW.hh>

#include <utm50_utils/linearalgebra.hh>

#include <vector>
#include <chrono>
#include <map>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include <assert.h>

namespace touchlib {

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
   * touchState.eventsInit(width, height);
   * touchState.setCurrentProjection(camera);
   *
   * SDL_Event event;
   * while(SDL_PollEvent(&event)) {
   *   touchState.getEventAdaptor<touchlib::SDLEventAdaptor>().handleEvent(event);
   * }
   * touchState.eventsDone();
   * 
   * TouchPoints touchPoints;
   * if (touchState.getTouchPoints(this, touchPoints) < REQUIRED_POINTS) {
   *   TouchPoints candidates;
   *   touchState.getTouchPoints(nullptr, candidates);
   *   for (auto& pt : candidates)
   *     if (isInside(pt.x, pt.y)) // Checks if the point hits this object
   *       touchState.setAssociation(pt.id, this);
   * }
   * 
   * TouchPoints previousTouchPoints;
   * if (touchState.getTouchPoints(this, touchPoints, previousTouchPoints) < REQUIRED_POINTS)
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

      /// Touch point was released.
      static const unsigned char RELEASE = 0x01 << 3;
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
      
      /// x velocity in pixels per second from left edge.
      float vy;

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
      utm50_utils::Vector4f x;

      /// The direction of the 3D line.
      utm50_utils::Vector3f v;
 
      /// internal id of the touch point.
      TouchPointId id;

      /**
       * State of the touch point expressed as a bitwise combination.
       * 
       * \see State
       */ 
      unsigned char state;
    };

    /**
     * List of touch lines.
     */
    typedef std::vector<TouchLine> TouchLines;
    
    /** @name Get touch points
     * Methods used to extract current touch points the their states.
     */
    ///@{
    
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
     * Gets the touch point id associated with a specified object.
     *
     * @param[in] id The id of the touch point that is associated.
     * @param[out] pt A void pointer to the object that is associated with.
     * @return True iff an association was found.
     */
    bool getAssociation(TouchPointId id, void* pt) const;

    ///!@}
    
    /** @name 3D projection methods
     * Methods used handle touch states' 3D behaviour.
     */
    ///@{

#ifdef TOUCHLIB_ENABLE_OpenSceneGraph
    /**
     * Calculates and sets the current projection matrix to use when
     * back-projecting the 2D touch points into 3D, and set the
     * previously current to be used when back-projecting "previous"
     * touch points.
     * 
     * If the view changes over time, one of these methods needs to be
     * called every frame, between calling eventsInit and eventsDone
     * and before extrating 3D lines. If the view is static, it needs
     * to be called at least twice, to make the current matrix be
     * copied to previous state. The frame copying is performed even
     * when the valid project cannot be extracted.
     * 
     * @return True if a valid projection could be extracted from the
     * specified camera, false otherwise.
     * 
     * \see getCurrent3DTouchLine
     */
    bool setCurrentProjection(osg::Camera * camera);
#endif
    
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
     * \see getCurrent3DTouchLine
     */
    void setCurrentProjection(utm50_utils::Matrix4f WPV_inv);
    
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
    bool getMouseLine(utm50_utils::Vector4f &x,
                      utm50_utils::Vector3f &v) const;
    
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
     * default 10) that is allowed before the touch point state DRAG
     * is applied. This is also the distance allowed between two or
     * more clicks for them to be considered a multi click. A negative
     * value turns this feature off, meaning that DRAG or MULTI are
     * never applied.
     *
     * @param[in] dist Euclidean distance in pixels
     */
    void setMoveMagnitude(float dist);
    
    /**
     * Gets the distance.
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

      /**
       * Called by the owner (TouchState) when eventsInit is called.
       */
      virtual void init(int width, int height) {}

      /**
       * Called by the owner (TouchState) when eventsDone is called.
       */
      virtual void done() {}

    protected:

      /**
       * Adds a touch point sample
       * @param id The id of the touch point
       * @param x position in (sub) pixels from left edge.
       * @param y position in (sub) pixels from top edge.
       * @param time the time of the event in seconds.
       */
      void addTouchState(TouchPointId id, float x, float y, double time);
      
      /**
       * Removes the touch point with the specified id.
       */
      void removeTouchState(TouchPointId id, float x, float y);

      /**
       * Add the specified mouse state. This will also add and remove
       * simulated touch states.
       */
      void addMouseState(TouchPointId id, float x, float y, double time, bool mouse_down);
      
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

    EFFOAW<utm50_utils::Vector3f> velocityEstimator;
    
    /**
     * Adds a touch point sample
     * @param id The id of the touch point
     * @param x position in (sub) pixels from left edge.
     * @param y position in (sub) pixels from top edge.
     * @param time the time of the event in seconds.
     */
    void addTouchState(TouchPointId id, float x, float y, double time);
    
    /**
     * Removes the touch point with the specified id.
     */
    void removeTouchState(TouchPointId id, float x, float y);
    
    /**
     * Add the specified mouse state. This will also add and remove
     * simulated touch states.
     */
    void addMouseState(TouchPointId id, float x, float y, double time, bool mouse_down);
    
    /**
     * Sets the wheel value, positive up and negative down.
     */
    void addMouseWheel(float s);
    
    /**
     * Internal
     */
    void addState(TouchPointId id, float x, float y, double time);
    
    float smoothing;
    float move_magnitude;
    clock::duration hold_time;
    clock::duration multi_time;
    
    bool remove_mouse_upon_touch;
    bool use_mouse;

    std::map<TouchPointId, TouchPoint> current_state;
    std::map<TouchPointId, TouchPoint> previous_state;
    
    std::map<TouchPointId, void*> association;
    std::map<TouchPointId, HistoryState> history;

    /**
     * Checks if this new touch point is really a multi click, based
     * on historical information.
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
     * Removes states that were previously released (has state
     * RELEASE). This is called from eventsInit().
     */
    void clearReleasedStates();

    int state;

    utm50_utils::Matrix4f current_WPV_inv;
    bool current_WPV_inv_valid;

    utm50_utils::Matrix4f previous_WPV_inv;
    bool previous_WPV_inv_valid;

    int current_height;
    int previous_height;
    int current_width;
    int previous_width;

    float mouse_wheel;
    bool mouse_down;
    int mouse_point_x;
    int mouse_point_y;

    std::unordered_map<std::type_index, EventAdaptor*> event_adaptors;
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
}

#endif
