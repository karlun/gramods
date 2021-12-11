
#ifndef GRAMODS_TOUCH_SDLEVENTADAPTOR
#define GRAMODS_TOUCH_SDLEVENTADAPTOR

#include <gmTouch/config.hh>

#ifdef gramods_ENABLE_SDL2

#include <gmTouch/TouchState.hh>

#include <SDL.h>

BEGIN_NAMESPACE_GMTOUCH;

/**
 * Event adaptor for SDL2, providing means to input events into
 * TouchState.
 *
 * Typical use:
 * \code{.cpp}
 * int width, height;
 * SDL_GetWindowSize(sdl_window, &width, &height);
 *
 * touchState.eventsInit(width, height);
 *
 * SDL_Event event;
 * while(SDL_PollEvent(&event)) {
 *   touchState.getEventAdaptor<gmTouch::SDLEventAdaptor>().handleEvent(event);
 * }
 * touchState.eventsDone();
 * \endcode
 */
class SDLEventAdaptor
  : public TouchState::EventAdaptor {

public:

  /**
   * Called by the owner (TouchState) when eventsInit is called.
   */
  void init(int width, int height) override;

  /**
   * Called by the owner (TouchState) when eventsDone is called.
   */
  void done() override;

  /**
   * Updates the internal touch states based on the provided
   * event. Call this once for each incoming event and finish with a
   * call to eventsDone.
   */
  void handleEvent(const SDL_Event& event);

private:

  int width, height;
  bool mouse_down = false;
};

END_NAMESPACE_GMTOUCH;

#endif
#endif
