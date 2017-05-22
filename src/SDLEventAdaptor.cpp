
#include <touchlib/SDLEventAdaptor.hh>

#ifdef TOUCHLIB_ENABLE_SDL2

using namespace touchlib;

void SDLEventAdaptor::init(int width, int height) {
  this->width = width;
  this->height = height;
}

void SDLEventAdaptor::done() {}

void SDLEventAdaptor::handleEvent(const SDL_Event& event) {
  switch (event.type) {
    
  case SDL_MOUSEMOTION:
    if (!mouse_down) return;
    
  case SDL_MOUSEBUTTONDOWN: {
    mouse_down = true;
    addState(event.motion.which, event.motion.x, event.motion.y,
             1e-3 * event.motion.timestamp, true);
    return;
  }
    
  case SDL_MOUSEBUTTONUP:
    mouse_down = false;
    removeState(event.motion.which, event.motion.x, event.motion.y, true);
    return;
    
  case SDL_FINGERDOWN: {
#ifdef __linux__
    float x = event.tfinger.x;
    float y = event.tfinger.y;
#else
    float x = event.tfinger.x * width;
    float y = event.tfinger.y * height;
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
    float x = event.tfinger.x * width;
    float y = event.tfinger.y * height;
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
