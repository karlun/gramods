
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

  CoordinatesMapper();
  virtual ~CoordinatesMapper();

  /**
     Returns shader "common" code, lines of code that are needed once
     regardless if 2D or 3D mapping code is used, or both.
  */
  virtual std::string getCommonCode() { return ""; }

  /**
     Returns shader code that implements the 3D to 2D mapping for the
     specific coordinates mapper.

     The code implements `bool mapTo2D(vec3 pos3d, out vec2 pos2d)`
     that calculates the 2D coordinates that correspond to the
     specified 3D position. Return true if pos2d is set to the 2D
     mapping of pos3d, or false if there is no such mapping.

     Argument pos3 is a unit 3D vector pointing in any direction, and
     pos2 is a 2D vector with each dimension ranging between -1 and 1,
     for left/bottom and right/top, respectively.
  */
  virtual std::string getTo2DCode() {
    throw gmCore::RuntimeException(
        "caller wants mapTo2D but this is not imlemented");
  }

  /**
     Returns shader code that implements the 2D to 3D mapping for the
     specific coordinates mapper.

     The code implements `bool mapTo3D(vec2 pos2d, out vec3 pos3d)`
     that calculates the 3D coordinates that correspond to the
     specified 2D position. Return true if pos3d is set to the 3D
     mapping of pos2d, or false if there is no such mapping.

     Argument pos2 is a 2D vector with each dimension ranging
     between -1 and 1, for left/bottom and right/top, respectively,
     and pos3 is a unit 3D vector pointing in any direction.
  */
  virtual std::string getTo3DCode() {
    throw gmCore::RuntimeException(
        "caller wants mapTo3D but this is not imlemented");
  }

  /**
     Called by the code that is using this CoordinatesMapper object,
     to let it set the uniforms used by the common code.
  */
  virtual void setCommonUniforms(GLuint program) {}

  /**
     Called by the code that is using this CoordinatesMapper object,
     to let it set the uniforms used by the to-2D code.
  */
  virtual void setTo2DUniforms(GLuint program) {}

  /**
     Called by the code that is using this CoordinatesMapper object,
     to let it set the uniforms used by the to-3D code.
  */
  virtual void setTo3DUniforms(GLuint program) {}

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  virtual std::string getDefaultKey() override { return "coordinatesMapper"; }

protected:
  const std::string withVarId(std::string) const;
  const std::string var_id;
};

END_NAMESPACE_GMGRAPHICS;

#endif
