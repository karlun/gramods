

#include "SDLWindow.hh"

#include <gmMisc/EFHOAW.hh>

#include <iostream>
#include <sstream>

#define POLY_ORDER  3
#define POLY_ERROR 20

SDLWindow::SDLWindow() :
  sdl_context(SDLContext::get()) {

  sdl_window = SDL_CreateWindow("gm-demo-end-fit", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);

  sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
  SDL_Renderer *renderer = sdl_renderer;

  alive = true;
  dirty = true;
}

SDLWindow::~SDLWindow() {
  if (sdl_renderer) SDL_DestroyRenderer(sdl_renderer);
  if (sdl_window) SDL_DestroyWindow(sdl_window);
}

bool realized = false;
void SDLWindow::process() {

  int width, height;
  SDL_GetWindowSize(sdl_window, &width, &height);

  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    handleEvent(event, width, height);
  }

  if (dirty)
    update();
}

void SDLWindow::update() {
  dirty = false;

  SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
  SDL_RenderClear(sdl_renderer);

  SDL_SetRenderDrawColor(sdl_renderer, 0, 255, 0, 255);
  for (auto pt : points)
    drawPoint(pt.x, pt.y);

  gramods::gmMisc::EFHOAW ef;
  ef.setHistoryLength(points.size());
  ef.setHistoryDuration(points.size());

  size_t idx = 0;
  for (auto pt : points)
    ef.addSample(0, Eigen::Vector3d(pt.x, pt.y, 0), idx++);

  try {
    auto coeffs = ef.estimateCoefficients(0, POLY_ERROR, POLY_ORDER);

    SDL_SetRenderDrawColor(sdl_renderer, 190, 190, 190, 255);

    auto pt = ef.getPolynomialPosition(0, 0);
    int x0 = int(pt[0]);
    int y0 = int(pt[1]);

    for (int idx = 0; idx < 2 * points.size(); ++idx) {

      if (idx + 1 < points.size())
        SDL_SetRenderDrawColor(sdl_renderer, 190, 255, 190, 255);
      else
        SDL_SetRenderDrawColor(sdl_renderer, 190, 190, 255, 255);

      for (int c = 0; c < 10; ++c) {

        double t = idx + 0.1 * c;
        auto pt = ef.getPolynomialPosition(0, t);

        int x1 = int(pt[0]);
        int y1 = int(pt[1]);

        SDL_RenderDrawLine(sdl_renderer, x0, y0, x1, y1);

        x0 = x1;
        y0 = y1;
      }
    }
  }
  catch(std::invalid_argument e) {
    std::cerr << e.what() << std::endl;
  }

  SDL_RenderPresent(sdl_renderer);
}

bool SDLWindow::handleEvent(SDL_Event& event, int width, int height) {
  static bool down = false;

  switch (event.type) {

  case SDL_MOUSEBUTTONDOWN:
    down = true;
    points.push_back(Point { event.motion.x, event.motion.y });
    dirty = true;
    return true;

  case SDL_MOUSEBUTTONUP:
    down = false;
    return true;

  case SDL_MOUSEMOTION:
    if (down) {
      points.back() = Point { event.motion.x, event.motion.y };
      dirty = true;
    }
    return true;

  case SDL_KEYDOWN:
    if (event.key.keysym.sym == SDLK_ESCAPE) {
      alive = false;
      return true;
    }
    return false;

  case SDL_QUIT:
    alive = false;
    return true;
  }

  return false;
}

void SDLWindow::drawPoint(int x, int y) {
  SDL_RenderDrawLine(sdl_renderer, x - 5, y - 5, x + 5, y + 5);
  SDL_RenderDrawLine(sdl_renderer, x + 5, y - 5, x - 5, y + 5);
}