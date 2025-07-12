
#include "SDLWindow.hh"

#include <gmMisc/PolyFit.hh>
#include <gmCore/InvalidArgument.hh>
#include <gmCore/Stringify.hh>

#include <tclap/CmdLine.h>

#include <iostream>
#include <sstream>

using namespace std::string_literals;

SDLWindow::SDLWindow(int argc, char *argv[]) : sdl_context(SDLContext::get()) {

  TCLAP::CmdLine cmd(
      "Demo for PolyFit (1D -> 2D). Click with mouse or touch to add samples. Input will be the input sample's index and output will be its position. Use 'c' or space to clear points and escape to exit.");

  TCLAP::ValueArg<size_t> arg_order(
      "o", "order", "Order of the polygonal approximation.", false, 2, "N");
  cmd.add(arg_order);

  try {
    cmd.parse(argc, argv);
  } catch (TCLAP::ArgException &e) {
    throw gramods::gmCore::InvalidArgument(
        GM_STR("error: " << e.error() << " for arg " << e.argId()));
  }

  order = arg_order.getValue();

  sdl_window = SDL_CreateWindow("gm-demo-polyfit", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE);
  sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);

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

  SDL_SetRenderDrawColor(sdl_renderer, 170, 170, 170, 255);
  for (auto pt : points) drawPoint(pt.x, pt.y);

  if (points.size() < 2) {
    SDL_RenderPresent(sdl_renderer);
    return;
  }

  gramods::gmMisc::PolyFit polyfit(1, 2, order);

  for (size_t idx = 0; idx < points.size(); ++idx)
    polyfit.addSample({double(idx)},
                      {double(points[idx].x), double(points[idx].y)});

  try {
    SDL_SetRenderDrawColor(sdl_renderer, 190, 190, 190, 255);

    auto pt = polyfit.getValue({-1.0});
    int x0 = int(pt[0]);
    int y0 = int(pt[1]);

    for (int idx = -1; idx < int(points.size()); ++idx) {

      if (idx < 0)
        SDL_SetRenderDrawColor(sdl_renderer, 190, 190, 255, 255);
      else
        SDL_SetRenderDrawColor(sdl_renderer, 190, 255, 190, 255);

      if (idx >= 0) {
        auto pt = polyfit.getValue({double(idx)});

        SDL_Rect rect;
        rect.x = int(pt[0]) - 2;
        rect.y = int(pt[1]) - 2;
        rect.w = 4;
        rect.h = 4;
        SDL_RenderDrawRect(sdl_renderer, &rect);
      }

      if (idx + 1 == points.size())
        SDL_SetRenderDrawColor(sdl_renderer, 190, 190, 255, 255);
      for (int c = 1; c <= 10; ++c) {

        double t = idx + 0.1 * c;
        auto pt = polyfit.getValue({t});

        int x1 = int(pt[0]);
        int y1 = int(pt[1]);

        SDL_RenderDrawLine(sdl_renderer, x0, y0, x1, y1);

        x0 = x1;
        y0 = y1;
      }
    }
  }
  catch (const gramods::gmCore::InvalidArgument &e) {
    std::cerr << e.what << std::endl;
  }

  SDL_RenderPresent(sdl_renderer);
}

bool SDLWindow::handleEvent(SDL_Event &event, int, int) {
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

  case SDL_TEXTINPUT:
    if (std::string(event.text.text) == "c"s ||
        std::string(event.text.text) == " "s) {
      points.clear();
      dirty = true;
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
