
#ifndef GRAMODS_GRAPHICS_ANGULARFISHEYECOORDINATESMAPPER
#define GRAMODS_GRAPHICS_ANGULARFISHEYECOORDINATESMAPPER

#include <gmGraphics/CoordinatesMapper.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Coordinates mapper that provides an angular fisheye mapping between
   2D and fully spherical 3D.
*/
class AngularFisheyeCoordinatesMapper
  : public gmGraphics::CoordinatesMapper {

public:

  AngularFisheyeCoordinatesMapper();
  virtual ~AngularFisheyeCoordinatesMapper();

  /**
     Returns shader code that implements the necessary functions for
     the specific coordinates mapper.

     The code implements

     - ```bool mapTo2D(vec3 pos3, out vec2 pos2)``` - calculates the
       2D coordinates that correspond to the specified 3D
       position. Return true if pos2 is set to the 2D mapping of pos3,
       false if there is no such mapping.

     - ```bool mapTo3D(vec2 pos2, out vec3 pos3)``` - calculates the
       3D coordinates that correspond to the specified 2D
       position. Return true if pos3 is set to the 3D mapping of pos2,
       false if there is no such mapping.
  */
  std::string getMapperCode();

  /**
     Called by the code that is using this CoordinatesMapper object,
     to let it set the uniforms used by the mapper code.
  */
  void setMapperUniforms(GLuint program_id);

  /**
     Set the vertical coverage that the 2D coordinates (y=[-1, 1])
     should have in the 3D sphere, as angle expressed as a value in
     radians between 0 and 2π. The typical fulldome format uses a
     coverage angle of π, which is also the default.
  */
  void setCoverageAngle(float a);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
