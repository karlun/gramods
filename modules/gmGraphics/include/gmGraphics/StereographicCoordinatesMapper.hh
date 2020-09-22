
#ifndef GRAMODS_GRAPHICS_STEREOGRAPHICCOORDINATESMAPPER
#define GRAMODS_GRAPHICS_STEREOGRAPHICCOORDINATESMAPPER

#include <gmGraphics/CoordinatesMapper.hh>

#include <gmTypes/angle.hh>

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
  void setMapperUniforms(GLuint program);

  /**
     Sets the radius of the stereographic sphere. In comparison, the
     2D coordinates range from -1 to 1. Default is 0.5.

     \b XML-attribute: \c radius
  */
  void setRadius(float R);

  /**
     Calculates and sets the radius of the stereographic sphere from
     the specified angle the projection from -1 to 1 should cover on
     the sphere. Default is π.

     \b XML-attribute: \c coverageAngle
  */
  void setCoverageAngle(gmTypes::angle a);

  /**
     Sets the yaw, the horizontal heading, a value in the range 0 to
     2π. Default is 0.

     \b XML-attribute: \c theta0

     \sa operator>>(std::istream &, gmTypes::angle &)
  */
  void setTheta0(gmTypes::angle a);

  /**
     Sets the pitch, the vertical heading, a value in the range -π/2
     to π/2. Default is π/2.

     \b XML-attribute: \c phi0

     \sa operator>>(std::istream &, gmTypes::angle &)
  */
  void setPhi0(gmTypes::angle a);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
