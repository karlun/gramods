
#include <gmGraphics/TiledView.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <deque>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(TiledView, View);
GM_OFI_PARAM(TiledView, tileLocation, gmCore::size4, TiledView::addTileLocation);
GM_OFI_POINTER(TiledView, view, gmGraphics::View, TiledView::addView);

struct TiledView::Impl {
  struct Tile {
    gmCore::size4 location;
    std::shared_ptr<View> view;
  };

  void addView(gmCore::size4 tile_location,
               std::shared_ptr<View> view);
  void renderFullPipeline(ViewSettings settings);
  std::deque<gmCore::size4> tile_locations;
  std::vector<Tile> tiles;
};

TiledView::TiledView()
  : _impl(std::make_unique<Impl>()) {}

void TiledView::renderFullPipeline(ViewSettings settings) {
  populateViewSettings(settings);
  _impl->renderFullPipeline(settings);
}

void TiledView::Impl::renderFullPipeline(ViewSettings settings) {

  size_t total_rows = 0;
  size_t total_cols = 0;

  for (auto tile : tiles) {
    total_rows = std::max(total_rows, tile.location[0] + tile.location[2]);
    total_cols = std::max(total_cols, tile.location[1] + tile.location[3]);
  }

  GLint cvp[4] = { 0, 0, 0, 0 };
  glGetIntegerv(GL_VIEWPORT, cvp);
  float col_width = cvp[2] / (float)total_cols;
  float row_height = cvp[3] / (float)total_rows;

  if (cvp[2] == 0 || cvp[3] == 0) {
    GM_ERR("TiledView", "Cannot render to degenerate viewport");
    return;
  }

  GM_VINF("TiledView",
          "Tiling "
          << "[" << total_rows
          << " " << total_cols
          << "] in ");
  GM_VINF("TiledView",
          "Tiling "
          << "(" << cvp[0]
          << " " << cvp[1]
          << " " << cvp[2]
          << " " << cvp[3] << ")");

  for (auto tile : tiles) {
    GLint tile_location_0 = total_rows - 1 - tile.location[0];
    GLint x = (GLint)(cvp[0] + tile.location[1] * col_width);
    GLint y = (GLint)(cvp[1] + tile_location_0  * row_height);
    GLsizei width =
      (tile.location[1] + tile.location[3] < total_cols)
      ? tile.location[3] * col_width
      : cvp[2] - tile.location[1] * col_width;
    GLsizei height =
      (tile_location_0 + tile.location[2] < total_rows)
      ? tile.location[2] * row_height
      : cvp[3] - tile_location_0 * row_height;

    GM_VVINF("TiledView",
             "Tile @ "
             << "[" << tile.location[0]
             << " " << tile.location[1]
             << " " << tile.location[2]
             << " " << tile.location[3]
             << "] -> "
             << "(" << x
             << " " << y
             << " " << width
             << " " << height << ")");

    glViewport(x, y, width, height);
    tile.view->renderFullPipeline(settings);
  }
}

void TiledView::addTileLocation(gmCore::size4 c) {
  _impl->tile_locations.push_back(c);
}

void TiledView::addView(std::shared_ptr<View> view) {

  if (_impl->tile_locations.empty())
    throw gmCore::InvalidArgument("tile location must be specified before adding view");

  auto tile_location = _impl->tile_locations.front();
  _impl->tile_locations.pop_front();

  if (tile_location[2] == 0 || tile_location[3] == 0)
    throw gmCore::InvalidArgument("tile location must specify a non-zero col and row span");

  _impl->addView(tile_location, view);
}

void TiledView::Impl::addView(gmCore::size4 tile_location,
                              std::shared_ptr<View> view) {
  tiles.push_back(Impl::Tile({ tile_location, view }));
}

void TiledView::clearRenderers(bool recursive) {
  if (recursive)
    for (auto tile : _impl->tiles)
      tile.view->clearRenderers(recursive);
  RendererDispatcher::clearRenderers(recursive);
}

END_NAMESPACE_GMGRAPHICS;
