
#ifndef GRAMODS_GRAPHICS_EQUIRECTANGULARCOORDINATESMAPPER
#define GRAMODS_GRAPHICS_EQUIRECTANGULARCOORDINATESMAPPER

#include <gmGraphics/CoordinatesMapper.hh>

#include <gmCore/angle.hh>

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

  EquirectangularCoordinatesMapper();
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
  void setMapperUniforms(GLuint program_id) override;

  /**
     Set the horizontal and vertical coverage angles, respectively,
     that the 2D coordinates (y=[-1, 1]) should have in the 3D sphere,
     as angle expressed as a value in radians between 0 and 2π for
     horizontal and between 0 and π for vertical. The typical
     equirectangular format uses a coverage angle of 2π and π, which
     is also the default.

     \gmXmlTag{gmGraphics,EquirectangularCoordinatesMapper,coverageAngle}

     \sa operator>>(std::istream &, gmCore::angle &)
  */
  void setCoverageAngle(gmCore::angle2 a);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
