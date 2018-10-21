
#ifndef GRAMODS_GRAPHICS_CUBEMAP
#define GRAMODS_GRAPHICS_CUBEMAP

#include <gmGraphics/config.hh>
#include <gmGraphics/RendererDispatcher.hh>
#include <gmGraphics/StereoscopicView.hh>

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

  CubeMap();
  ~CubeMap();

  /**
     Renders the cube map.
  */
  void renderFullPipeline(std::vector<std::shared_ptr<Renderer>> renderers,
                          Eigen::Vector3f pos,
                          Eigen::Quaternionf rot);

  /**
     Returns the shader program, for setting uniforms.
  */
  int getProgram();

  /**
     Sets the fragment code
  */
  void setFragmentCode(std::string code);

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

  /**
     Sets the position and dimensions of a spatial cube map, for
     example for a spatial dome rendering.
  */
  void setSpatialCubeMap(Eigen::Vector3f c, float side);

private:

  class Impl;
  Impl * _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
