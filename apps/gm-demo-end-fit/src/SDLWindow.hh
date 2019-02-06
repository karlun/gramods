
#ifndef __SDLWINDOW_HH__
#define __SDLWINDOW_HH__

#include "SDLContext.hh"

#include <SDL_render.h>

#include <chrono>
#include <memory>
#include <vector>

class SDLWindow {

public:

  /// The clock used in this class.
  typedef std::chrono::steady_clock clock;

  struct Point {
    int x;
    int y;
  };

  /**
   * Creates a default window.
   */
  SDLWindow();

  /** Cleanup. */
  virtual ~SDLWindow();

  /**
   * Handles SDL events internally.
   */
  bool handleEvent(SDL_Event& event, int width, int height);

  /**
   * Called by governing thread to update the states of this window,
   * for example handling events and rendering graphics.
   */
  void process();

  void update();

  /**
   *
   */
  bool isDone() { return !alive; }

protected:

  SDL_Window *sdl_window;
  SDL_Renderer *sdl_renderer;

  std::vector<Point> points;

  bool alive;
  bool dirty;

private:

  /** Pointer to the SDL Context singleton for initializing and
      destroying the SDL context. */
  std::shared_ptr<SDLContext> sdl_context;

  void drawPoint(int x, int y);

};

#endif
