
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
    
  case SDL_MOUSEWHEEL:
    addMouseWheel(event.wheel.y);
    return;
    
  case SDL_MOUSEMOTION:
    addMouseState(event.motion.which, event.motion.x, event.motion.y,
                  1e-3 * event.motion.timestamp, event.motion.state);
    return;
    
  case SDL_MOUSEBUTTONUP:
  case SDL_MOUSEBUTTONDOWN:
    addMouseState(event.button.which, event.button.x, event.button.y,
                  1e-3 * event.button.timestamp, event.button.state);
    return;
    
  case SDL_FINGERDOWN: {
#ifdef __linux__
    float x = event.tfinger.x;
    float y = event.tfinger.y;
#else
    float x = event.tfinger.x * width;
    float y = event.tfinger.y * height;
#endif
    addTouchState(event.tfinger.fingerId, x, y,
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
    addTouchState(event.tfinger.fingerId, x, y, 1e-3 * event.tfinger.timestamp);
    return;
  }
    
  case SDL_FINGERUP:
#ifdef __linux__
    float x = event.tfinger.x;
    float y = event.tfinger.y;
#else
    float x = event.tfinger.x * width;
    float y = event.tfinger.y * height;
#endif
    removeTouchState(event.tfinger.fingerId, x, y);
    return;
  }
}
#endif
