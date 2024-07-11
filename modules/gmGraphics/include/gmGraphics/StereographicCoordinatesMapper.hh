
#ifndef GRAMODS_GRAPHICS_STEREOGRAPHICCOORDINATESMAPPER
#define GRAMODS_GRAPHICS_STEREOGRAPHICCOORDINATESMAPPER

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_angle.hh>

#include <gmGraphics/CoordinatesMapper.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Coordinates mapper that provides a stereographic mapping between
   2D and 3D.

   The stereographic projection is conformal. As such it preserves the
   angles at which curves cross each other, but on the other hand does
   not preserve area.
*/
class StereographicCoordinatesMapper
  : public gmGraphics::CoordinatesMapper {

public:

  StereographicCoordinatesMapper();
  virtual ~StereographicCoordinatesMapper();

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
     Sets the radius of the stereographic sphere. In comparison, the
     2D coordinates range from -1 to 1. Default is 0.5.

     \gmXmlTag{gmGraphics,StereographicCoordinatesMapper,radius}
  */
  void setRadius(float R);

  /**
     Calculates and sets the radius of the stereographic sphere from
     the specified angle the projection from -1 to 1 should cover on
     the sphere. Default is π.

     \gmXmlTag{gmGraphics,StereographicCoordinatesMapper,coverageAngle}
  */
  void setCoverageAngle(gmCore::angle a);

  /**
     Sets the yaw, the horizontal heading, a value in the range 0 to
     2π. Default is 0.

     \gmXmlTag{gmGraphics,StereographicCoordinatesMapper,theta0}

     \sa operator>>(std::istream &, gmCore::angle &)
  */
  void setTheta0(gmCore::angle a);

  /**
     Sets the pitch, the vertical heading, a value in the range -π/2
     to π/2. Default is π/2.

     \gmXmlTag{gmGraphics,StereographicCoordinatesMapper,phi0}

     \sa operator>>(std::istream &, gmCore::angle &)
  */
  void setPhi0(gmCore::angle a);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
