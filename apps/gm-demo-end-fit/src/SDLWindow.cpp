

#include "SDLWindow.hh"

#include <gmMisc/EFHOAW.hh>
#include <gmCore/InvalidArgument.hh>
#include <gmCore/Stringify.hh>

#include <tclap/CmdLine.h>

#include <iostream>
#include <sstream>

SDLWindow::SDLWindow(int argc, char *argv[]) : sdl_context(SDLContext::get()) {

  TCLAP::CmdLine cmd(
      "Demo for PolyFit (1D -> 2D). Click with mouse or touch to add samples. Input will be the input sample's index and output will be its position. Use 'c' or space to clear points and escape to exit.");

  TCLAP::ValueArg<size_t> arg_order(
      "o", "order", "Order of the polygonal approximation. Default is 2.", false, 2, "N");
  cmd.add(arg_order);
  TCLAP::ValueArg<size_t> arg_history_count(
      "c",
      "history-count",
      "Number of historical samples to take into consideration. Default is 10.",
      false,
      10,
      "N");
  cmd.add(arg_history_count);
  TCLAP::ValueArg<double> arg_poly_error(
      "e",
      "error",
      "Allowed error in the polygonal approximation. Default is 20.",
      false,
      20.0,
      "N");
  cmd.add(arg_poly_error);

  try {
    cmd.parse(argc, argv);
  } catch (TCLAP::ArgException &e) {
    throw gramods::gmCore::InvalidArgument(
        GM_STR("error: " << e.error() << " for arg " << e.argId()));
  }

  poly_order = arg_order.getValue();
  history_count = arg_history_count.getValue();
  poly_error = arg_poly_error.getValue();

  sdl_window = SDL_CreateWindow("gm-demo-end-fit", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE);
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
  for (auto pt : points)
    drawPoint(pt.x, pt.y);

  gramods::gmMisc::EFHOAW ef;
  ef.setHistoryLength(history_count);
  ef.setHistoryDuration(points.size());

  size_t idx = 0;
  for (auto pt : points)
    ef.addSample(0, Eigen::Vector3d(pt.x, pt.y, 0), idx++);

  try {
    size_t sample_count;
    auto coeffs = ef.estimateCoefficients(0, poly_error, poly_order, &sample_count);

    SDL_SetRenderDrawColor(sdl_renderer, 190, 190, 190, 255);

    auto pt = ef.getPolynomialPosition(0, points.size() - sample_count);
    int x0 = int(pt[0]);
    int y0 = int(pt[1]);

    for (size_t idx = points.size() - sample_count; idx < points.size() + 2; ++idx) {

      if (idx + 1 < points.size())
        SDL_SetRenderDrawColor(sdl_renderer, 190, 255, 190, 255);
      else
        SDL_SetRenderDrawColor(sdl_renderer, 190, 190, 255, 255);

      {
        auto pt = ef.getPolynomialPosition(0, idx);

        SDL_Rect rect;
        rect.x = int(pt[0]) - 2;
        rect.y = int(pt[1]) - 2;
        rect.w = 4;
        rect.h = 4;
        SDL_RenderDrawRect(sdl_renderer, &rect);
      }

      for (int c = 1; c <= 10; ++c) {

        double t = idx + 0.1 * c;
        auto pt = ef.getPolynomialPosition(0, t);

        int x1 = int(pt[0]);
        int y1 = int(pt[1]);

        SDL_RenderDrawLine(sdl_renderer, x0, y0, x1, y1);

        x0 = x1;
        y0 = y1;
      }
    }

    SDL_SetRenderDrawColor(sdl_renderer, 0, 255, 0, 255);
    for (size_t idx = points.size() - sample_count; idx < points.size(); ++idx)
      drawPoint(points[idx].x, points[idx].y);

  }
  catch (const gramods::gmCore::InvalidArgument &e) {
    std::cerr << "Error: " << e.what << std::endl;
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
