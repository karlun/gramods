
#ifndef __TOUCHLIB_SDLEVENTADAPTOR_HH__
#define __TOUCHLIB_SDLEVENTADAPTOR_HH__

#include <touchlib/config.hh>

#ifdef TOUCHLIB_ENABLE_SDL2

#include <touchlib/TouchState.hh>

#include <SDL.h>

namespace touchlib {
  
  /**
   *
   */
  class SDLEventAdaptor
    : public TouchState::EventAdaptor {
    
  public:

    /**
     * Called by the owner (TouchState) when eventsInit is called.
     */
    void init(int width, int height);
    
    /**
     * Called by the owner (TouchState) when eventsDone is called.
     */
    void done();

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
  
}

#endif
#endif
