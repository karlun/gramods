
#ifndef __SDLCONTEXT_HH__
#define __SDLCONTEXT_HH__

#include <SDL.h>

#include <memory>

/**
 * The SDLContext class provides a singleton encoding of the SDL
 * initialization and cleanup. By using a weak singleton pointer,
 * this class will only keep the context alive as long as there is
 * anyone using it.
 */
class SDLContext {

  SDLContext();

public:

  static std::shared_ptr<SDLContext> get();

  ~SDLContext();

protected:

  static std::weak_ptr<SDLContext> sdl_context;

};

#endif
