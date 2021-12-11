
#ifndef GRAMODS_GRAPHICS_EQUIRECTANGULARCOORDINATESMAPPER
#define GRAMODS_GRAPHICS_EQUIRECTANGULARCOORDINATESMAPPER

#include <gmGraphics/CoordinatesMapper.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Coordinates mapper that provides an equirectangular mapping between
   2D and fully spherical 3D.
*/
class EquirectangularCoordinatesMapper
  : public gmGraphics::CoordinatesMapper {

public:

  virtual ~EquirectangularCoordinatesMapper();

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
  std::string getMapperCode() override;

  GM_OFI_DECLARE;
};

END_NAMESPACE_GMGRAPHICS;

#endif
