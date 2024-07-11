
#ifndef GRAMODS_GRAPHICS_ANGULARFISHEYECOORDINATESMAPPER
#define GRAMODS_GRAPHICS_ANGULARFISHEYECOORDINATESMAPPER

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_angle.hh>

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
     Set the vertical coverage that the 2D coordinates (y=[-1, 1])
     should have in the 3D sphere, as angle expressed as a value in
     radians between 0 and 2π. The typical fulldome format uses a
     coverage angle of π, which is also the default.

     \gmXmlTag{gmGraphics,AngularFisheyeCoordinatesMapper,coverageAngle}

     \sa operator>>(std::istream &, gmCore::angle &)
  */
  void setCoverageAngle(gmCore::angle a);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
