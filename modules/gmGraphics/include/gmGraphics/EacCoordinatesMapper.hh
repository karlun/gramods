
#ifndef GRAMODS_GRAPHICS_EACCOORDINATESMAPPER
#define GRAMODS_GRAPHICS_EACCOORDINATESMAPPER

#include <gmGraphics/CoordinatesMapper.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Coordinates mapper that provides an Equi-angular Cubemap (EAC)
   mapping between 2D and fully spherical 3D.
*/
class EacCoordinatesMapper
  : public gmGraphics::CoordinatesMapper {

public:

  EacCoordinatesMapper();
  virtual ~EacCoordinatesMapper();

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

     For both functions pos2 is a 2D vector with each dimension
     ranging between -1 and 1, for left/bottom and right/top,
     respectively, and pos3 is a unit 3D vector pointing in any
     direction.
  */
  std::string getMapperCode() override;

  /**
     Called by the code that is using this CoordinatesMapper object,
     to let it set the uniforms used by the mapper code.
  */
  void setMapperUniforms(GLuint program) override;

  /**
     Sets the cubemap layout through a string of row-wise cube sides,
     optionally rotated, and row-break(s).

     Valid characters are

     - L R U D F B for left, right, up, down, front, back,
       respectively
     - / for a new row
     - - + for rotating the last square left or right, respectively

     Some example layouts are

     - LFR/D-B-U- (upper row is left-to-right; default)
     - LFR/U+B+D+ (same, but lower row reversed)
     - L-U/F-B/R-D (columns instead of rows)

     \gmXmlTag{gmGraphics,EacCoordinatesMapper,layout}
  */
  void setLayout(std::string layout);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
