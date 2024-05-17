
#ifndef GRAMODS_GRAPHICS_CUBEMAPRASTERPROCESSOR
#define GRAMODS_GRAPHICS_CUBEMAPRASTERPROCESSOR

#include <gmGraphics/config.hh>
#include <gmGraphics/ViewBase.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <memory>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The CubeMapRasterProcessor is a helper class for views that map the
   surrounding to a flat format. One call to renderFullPipeline will
   make up to six cube map side rendering calls to the renderers
   specified in the ViewSettings, as well as a final rendering through
   the specified fragment shader code.
*/
class CubeMapRasterProcessor {

public:

  CubeMapRasterProcessor();
  ~CubeMapRasterProcessor();

  /**
     Renders the cube map.
  */
  void renderFullPipeline(ViewBase::ViewSettings &settings,
                          Eigen::Vector3f pos,
                          Eigen::Quaternionf rot,
                          Eye eye,
                          bool make_square = false);

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
     Set the pixel format to use for the color buffer(s). Default is
     GL_RGBA8.
  */
  void setPixelFormat(GLenum format);

  /**
     Return the pixel format currently used for the color buffer(s).
  */
  GLenum getPixelFormat();

  /**
     Sets the position and dimensions of a spatial cube map, for
     example for a spatial dome rendering.
  */
  void setSpatialCubeMap(Eigen::Vector3f c, float side);

private:

  struct Impl;
  Impl * _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
