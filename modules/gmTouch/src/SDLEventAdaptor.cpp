
#include <gmTouch/SDLEventAdaptor.hh>

#ifdef gramods_ENABLE_SDL2

BEGIN_NAMESPACE_GMTOUCH;

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
                  1e-3 * event.motion.timestamp, mouse_down);
    return;
    
  case SDL_MOUSEBUTTONUP:
  case SDL_MOUSEBUTTONDOWN:
    if (event.button.button != SDL_BUTTON_LEFT) return;
    mouse_down = event.button.state;
    addMouseState(event.button.which, event.button.x, event.button.y,
                  1e-3 * event.button.timestamp, mouse_down);
    return;
    
  case SDL_FINGERDOWN: {
    float x = event.tfinger.x * width;
    float y = event.tfinger.y * height;
    addTouchState(event.tfinger.fingerId, x, y,
                  1e-3 * event.tfinger.timestamp);
    return;
  }
    
  case SDL_FINGERMOTION: {
    float x = event.tfinger.x * width;
    float y = event.tfinger.y * height;
    addTouchState(event.tfinger.fingerId, x, y, 1e-3 * event.tfinger.timestamp);
    return;
  }
    
  case SDL_FINGERUP:
    float x = event.tfinger.x * width;
    float y = event.tfinger.y * height;
    removeTouchState(event.tfinger.fingerId, x, y);
    return;
  }
}

END_NAMESPACE_GMTOUCH;

#endif
