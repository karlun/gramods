
#ifndef GRAMODS_GRAPHICS_CUBEMAP
#define GRAMODS_GRAPHICS_CUBEMAP

#include <gmGraphics/config.hh>
#include <gmGraphics/RendererDispatcher.hh>

#include <memory>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The CubeMap is a helper class for views that map the surrounding to
   a flat format. One call to renderFullPipeline will make up to six
   cube map side rendering calls to the renderers specified in the
   ViewSettings, as well as a final rendering through the specified
   fragment shader code.
*/
class CubeMap {

public:

  CubeMap(std::string fragment_code);
  ~CubeMap();

  /**
     Renders the cube map.
  */
  void renderFullPipeline(RendererDispatcher::ViewSettings settings);

  /**
     Returns the shader program, for setting uniforms.
  */
  int getProgram();

  /**
     Sets the resolution of the intermediate cube map, in
     pixels. Default is 2048.
  */
  void setCubeMapResolution(int res);

  /**
     Turn on or off linear interpolation in the projection from linear
     graphics to non-linear projection space. Default is off.
  */
  void setLinearInterpolation(bool on);

private:

  class Impl;
  Impl * _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
