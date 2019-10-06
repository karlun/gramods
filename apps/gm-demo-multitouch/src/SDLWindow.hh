
#ifndef __SDLWINDOW_HH__
#define __SDLWINDOW_HH__

#include <gmTouch/TouchState.hh>

#include "SDLContext.hh"

#include <SDL_render.h>
#include <SDL_ttf.h>

#include <chrono>
#include <memory>
#include <map>

namespace gramods {
  
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

    /**
     * 
     */
    bool isDone() { return !alive; }
    
  protected:
    
    SDL_Window *sdl_window;
    SDL_Renderer *sdl_renderer;
    TTF_Font *font;

    gmTouch::TouchState touchState;
    gmTouch::TouchState::TouchPoint releasedPoint;
    bool have_released_point;
    
    bool alive;
    
  private:
    
    /** Pointer to the SDL Context singleton for initializing and
        destroying the SDL context. */
    std::shared_ptr<SDLContext> sdl_context;

    void drawPoint(const gmTouch::TouchState::TouchPoint &pt,
                   int radius, std::string text, int pos);
    void drawLine(const Point &ptA, const Point &ptB, int R, int G, int B);
    void drawCircle(float px, float py, int radius);

    static std::string pointToString(gmTouch::TouchState::TouchPoint pt);

  };
  
}

#endif
