
#ifndef GRAMODS_GRAPHICS_COORDINATESMAPPER
#define GRAMODS_GRAPHICS_COORDINATESMAPPER

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>

#include <gmGraphics/Camera.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   A CoordinatesMapper provides a mapping between 2D and 3D.

   The 2D coordinates are ranged -1 to 1 and the 3D coordinates should
   be a unit vector being the direction corresponding to a point in
   the 2D space.
*/
class CoordinatesMapper
  : public gmCore::Object {

public:

  virtual ~CoordinatesMapper();

  /**
     Returns shader code that implements the necessary functions for
     the specific coordinates mapper.

     The code must implement one or both of

     - ```bool mapTo2D(vec3 pos3d, out vec2 pos2d)``` - calculates the
       2D coordinates that correspond to the specified 3D
       position. Return true if pos2d is set to the 2D mapping of
       pos3d, or false if there is no such mapping.

     - ```bool mapTo3D(vec2 pos2d, out vec3 pos3d)``` - calculates the
       3D coordinates that correspond to the specified 2D
       position. Return true if pos3d is set to the 3D mapping of
       pos2d, or false if there is no such mapping.
  */
  virtual std::string getMapperCode() { return ""; }

  /**
     Called by the code that is using this CoordinatesMapper object,
     to let it set the uniforms used by the mapper code.
  */
  virtual void setMapperUniforms(GLuint program) {}

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  virtual std::string getDefaultKey() { return "coordinatesMapper"; }

};

END_NAMESPACE_GMGRAPHICS;

#endif
