
#ifndef GRAMODS_GRAPHICS_EQUIRECTANGULARVIEW
#define GRAMODS_GRAPHICS_EQUIRECTANGULARVIEW

#include <gmGraphics/View.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The EquirectangularView maps the renderers' graphics into an
   equirectangular projection, also known as lat-long projection.
*/
class EquirectangularView
  : public View {

public:

  EquirectangularView();

  /**
     Dispatches renderers.
  */
  void renderFullPipeline(ViewSettings settings);

  /**
     Sets the resolution of the intermediate cube map, in
     pixels. Default is 2048.
  */
  void setCubeMapResolution(int res);

  GM_OFI_DECLARE;

private:

  class Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
