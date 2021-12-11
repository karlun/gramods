
#ifndef GRAMODS_TOUCH_TUIOEVENTADAPTOR
#define GRAMODS_TOUCH_TUIOEVENTADAPTOR

#include <gmTouch/config.hh>

#ifdef gramods_ENABLE_TUIO

#include <gmTouch/TouchState.hh>

#include <memory>
#include <mutex>

BEGIN_NAMESPACE_GMTOUCH;

/**
 * Event adaptor for TUIO, providing means to input events into
 * TouchState.
 *
 * Typical use:
 * \code{.cpp}
 * touchState.getEventAdaptor<touchlib::TUIOEventAdaptor>().open();
 * ...
 * int width, height;
 * SDL_GetWindowSize(sdl_window, &width, &height);
 *
 * touchState.eventsInit(width, height);
 * touchState.eventsDone();
 * \endcode
 */
class TUIOEventAdaptor
  : public TouchState::EventAdaptor {

public:
  TUIOEventAdaptor();

  /**
   * Connects the TUIO client with TCP to the specified address
   * (localhost if not specified) and port (3333 if not
   * specified).
   */
  void connect(std::string address = "localhost", int port = 3333);

  /**
   * Opens the TUIO client to listen to UDP packages on the
   * specified port (3333 if not specified).
   */
  void open(int port = 3333);

  /**
   * Called by the owner (TouchState) when eventsInit is called.
   */
  void init(int width, int height) override;

  /**
   * Synchronizes touch events from Tuio to the touch state. Called
   * by the owner (TouchState) when eventsDone is called.
   */
  void done() override;

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMTOUCH;

#endif
#endif
