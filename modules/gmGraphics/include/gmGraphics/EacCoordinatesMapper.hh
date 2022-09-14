
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
     @see CoordinatesMapper
  */
  void setTo2DUniforms(GLuint program) override;

  /**
     @see CoordinatesMapper
  */
  void setTo3DUniforms(GLuint program) override;

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
