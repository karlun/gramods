
#include <gmGraphics/TiledView.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <deque>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(TiledView, View);
GM_OFI_PARAM2(TiledView, tileLocation, gmCore::size4, addTileLocation);
GM_OFI_POINTER2(TiledView, view, gmGraphics::View, addView);

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
  float col_width = float(cvp[2]) / float(total_cols);
  float row_height = float(cvp[3]) / float(total_rows);

  if (cvp[2] == 0 || cvp[3] == 0) {
    GM_ERR("TiledView", "Cannot render to degenerate viewport");
    return;
  }

  GM_DBG2("TiledView",
          "Tiling "
          << "[" << total_rows
          << " " << total_cols
          << "] in ");
  GM_DBG2("TiledView",
          "Tiling "
          << "(" << cvp[0]
          << " " << cvp[1]
          << " " << cvp[2]
          << " " << cvp[3] << ")");

  for (auto tile : tiles) {
    GLint tile_location_0 = total_rows - tile.location[2] - tile.location[0];
    GLint x = (GLint)(cvp[0] + tile.location[1] * col_width);
    GLint y = (GLint)(cvp[1] + tile_location_0 * row_height);
    GLsizei width = (tile.location[1] + tile.location[3] < total_cols)
                        ? GLsizei(tile.location[3] * col_width)
                        : GLsizei(cvp[2] - tile.location[1] * col_width);
    GLsizei height = (tile_location_0 + tile.location[2] < total_rows)
                         ? GLsizei(tile.location[2] * row_height)
                         : GLsizei(cvp[3] - tile_location_0 * row_height);

    GM_DBG3("TiledView",
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

  glViewport(cvp[0], cvp[1], cvp[2], cvp[3]);
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
  if (!view) throw gmCore::InvalidArgument("null not allowed");
  tiles.push_back(Impl::Tile({ tile_location, view }));
}

void TiledView::traverse(Visitor *visitor) {
  for (auto &t : _impl->tiles) t.view->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
