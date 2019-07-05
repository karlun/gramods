

#include "SDLWindow.hh"

#include <gmTouch/SDLEventAdaptor.hh>

#include <iostream>
#include <sstream>

using namespace gramods;

SDLWindow::SDLWindow() :
  sdl_context(SDLContext::get()) {
  
  touchState.getEventAdaptor<gmTouch::SDLEventAdaptor>();
  
  sdl_window = SDL_CreateWindow("gm-demo-multitouch", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);

  sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);

  if (TTF_Init() < 0) {
    std::cerr << "Could not init SDL TTF" << std::endl;
  }
  
  font = TTF_OpenFont("/usr/share/fonts/truetype/msttcorefonts/times.ttf", 24);
  if (font == NULL)
    std::cerr << TTF_GetError() << std::endl;

  alive = true;
  have_released_point = false;
}

SDLWindow::~SDLWindow() {
  if (sdl_renderer) SDL_DestroyRenderer(sdl_renderer);
  if (sdl_window) SDL_DestroyWindow(sdl_window);
}

bool realized = false;
void SDLWindow::process() {
  
  int width, height;
  SDL_GetWindowSize(sdl_window, &width, &height);
  
  SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
  SDL_RenderClear(sdl_renderer);

  touchState.eventsInit(width, height);
  
  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    handleEvent(event, width, height);
    touchState.getEventAdaptor<gmTouch::SDLEventAdaptor>().handleEvent(event);
  }

  touchState.eventsDone();

  gmTouch::TouchState::TouchPoints current;
  touchState.getTouchPoints(current);

  for (size_t idx = 0; idx < current.size(); ++idx) {
    Point
      A = { int(current[idx].x),
            int(current[idx].y) };

    drawPoint(A, 50, pointToString(current[idx]), 1);

    if (current[idx].state & gmTouch::TouchState::State::RELEASE) {
      releasedPoint = current[idx];
      have_released_point = true;
    }
  }

  if (have_released_point) {
    Point
      C = { int(releasedPoint.x),
            int(releasedPoint.y) };

    drawPoint(C, 50, pointToString(releasedPoint), 3);
  }

  SDL_RenderPresent(sdl_renderer);
}

std::string SDLWindow::pointToString(gmTouch::TouchState::TouchPoint pt) {
  std::stringstream str;
  str << "TS" << pt.id;
  if (pt.state & gmTouch::TouchState::State::DRAG)
    str << "/DRAG";
  if (pt.state & gmTouch::TouchState::State::HOLD)
    str << "/HOLD";
  if (pt.state & gmTouch::TouchState::State::MULTI)
    str << "/MULTI(" << pt.clicks << ")";
  if (pt.state & gmTouch::TouchState::State::RELEASE)
    str << "/RELEASE";
  if (pt.state & gmTouch::TouchState::State::CLICK)
    str << "/CLICK";
  return str.str();
}

bool SDLWindow::handleEvent(SDL_Event& event, int width, int height) {

  switch (event.type) {
    
  case SDL_KEYDOWN:

    switch (event.key.keysym.sym) {

    case SDLK_ESCAPE:
      alive = false;
      return true;

    case SDLK_F11:
      if (SDL_GetWindowFlags(sdl_window)&SDL_WINDOW_FULLSCREEN_DESKTOP)
        SDL_SetWindowFullscreen(sdl_window, 0);
      else
        SDL_SetWindowFullscreen(sdl_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
      return true;
    }

    return false;
    
  case SDL_QUIT:
    alive = false;
    return true;
  }
  
  return false;
}

void SDLWindow::drawPoint(const Point &pt, int radius, std::string text, int pos) {
  
  SDL_SetRenderDrawColor(sdl_renderer, 255, 0, 0, 255);
  SDL_Color color = {64, 64, 255};
  
  int dx = 0;
  int dy = 0;
  
  switch (pos) {
    
  case 0:
    dx = 2 * radius;
    dy = 2 * radius;
    SDL_SetRenderDrawColor(sdl_renderer, 64, 64, 255, 255);
    color = {64, 64, 255};
    break;
    
  case 1:
    dx = 2 * radius;
    dy = -2 * radius;
    SDL_SetRenderDrawColor(sdl_renderer, 64, 255, 64, 255);
    color = {64, 255, 64};
    break;
    
  case 2:
    dx = 2 * radius;
    dy = 0;
    SDL_SetRenderDrawColor(sdl_renderer, 128, 128, 128, 255);
    color = {128, 128, 128};
    break;
    
  case 3:
    dx = 2 * radius;
    dy = 2 * radius;
    SDL_SetRenderDrawColor(sdl_renderer, 255, 64, 64, 255);
    color = {255, 64, 64};
    break;
    
  }

  if (radius > 0) {

    int x0 = pt.x + radius;
    int y0 = pt.y;

    for (int idx = 1; idx < 64; ++idx) {

      int x1 = pt.x + radius * cos((2 * 3.1416 / 64.0) * idx);
      int y1 = pt.y + radius * sin((2 * 3.1416 / 64.0) * idx);

      SDL_RenderDrawLine(sdl_renderer, x0, y0, x1, y1);

      x0 = x1;
      y0 = y1;
    }

    SDL_RenderDrawLine(sdl_renderer, x0, y0, pt.x + radius, pt.y);
  }

  if (font == nullptr)
    return;

  SDL_Surface* surfaceMessage = TTF_RenderText_Solid(font, text.c_str(), color);
  SDL_Texture* Message = SDL_CreateTextureFromSurface(sdl_renderer, surfaceMessage);
  
  SDL_Rect Message_rect;
  Message_rect.x = pt.x + dx;
  Message_rect.y = pt.y + dy - surfaceMessage->h / 2;
  Message_rect.w = surfaceMessage->w;
  Message_rect.h = surfaceMessage->h;
  
  SDL_RenderCopy(sdl_renderer, Message, NULL, &Message_rect);

  SDL_DestroyTexture(Message);
  SDL_FreeSurface(surfaceMessage);
  
  SDL_RenderDrawLine(sdl_renderer, pt.x, pt.y, pt.x + ((8*dx)/10), pt.y + ((8*dy)/10));
}

void SDLWindow::drawLine(const Point &ptA, const Point &ptB, int R, int G, int B) {
  SDL_SetRenderDrawColor(sdl_renderer, R, G, B, 255);
  SDL_RenderDrawLine(sdl_renderer, ptA.x, ptA.y, ptB.x, ptB.y);
}
