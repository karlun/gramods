
#ifndef GRAMODS_GRAPHICS_EQUIRECTANGULARCOORDINATESMAPPER
#define GRAMODS_GRAPHICS_EQUIRECTANGULARCOORDINATESMAPPER

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_angle.hh>

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

  EquirectangularCoordinatesMapper();
  virtual ~EquirectangularCoordinatesMapper();

  /**
     @see CoordinatesMapper
  */
  std::string getCommonCode() override;

  /**
     @see CoordinatesMapper
  */
  std::string getTo2DCode() override;

  /**
     @see CoordinatesMapper
  */
  std::string getTo3DCode() override;

  /**
     @see CoordinatesMapper
  */
  void setCommonUniforms(GLuint program) override;

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

  /**
     Set the horizontal and vertical coverage ranges, respectively,
     that the 2D coordinates (y=[-1, 1]) should have in the 3D sphere,
     as angle expressed as a value in radians typically between -π and
     π for horizontal and between -π/2 and π/2 for vertical. The
     typical equirectangular format uses a coverage range of -π - π
     and -π/2 - π/2, which is also the default.

     \gmXmlTag{gmGraphics,EquirectangularCoordinatesMapper,coverageRange}

     \sa operator>>(std::istream &, gmCore::angle &)
  */
  void setCoverageRange(gmCore::angle4 a);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
