
#include "SDLContext.hh"

#include <iostream>


std::weak_ptr<SDLContext> SDLContext::sdl_context;

SDLContext::SDLContext() {
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "Unable to init SDL: " << SDL_GetError() << std::endl;
    exit(1);
  }
}

SDLContext::~SDLContext() {
  SDL_Quit();
}

std::shared_ptr<SDLContext> SDLContext::get() {
  if (auto ctx = sdl_context.lock())
    return ctx;

  std::shared_ptr<SDLContext> ptr(new SDLContext);
  sdl_context = ptr;
  return ptr;
}
