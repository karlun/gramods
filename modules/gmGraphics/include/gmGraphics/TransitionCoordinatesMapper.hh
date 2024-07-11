
#ifndef GRAMODS_GRAPHICS_TRANSITIONCOORDINATESMAPPER
#define GRAMODS_GRAPHICS_TRANSITIONCOORDINATESMAPPER

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_float.hh>
#include <gmCore/io_size.hh>

#include <gmGraphics/CoordinatesMapper.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Coordinates mapper that provides a transition between two other
   coordinates mappers.
*/
class TransitionCoordinatesMapper
  : public gmGraphics::CoordinatesMapper {

public:

  TransitionCoordinatesMapper();
  virtual ~TransitionCoordinatesMapper();

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
     Set the start and end time at which the transition should be
     made. The first mapper is used up until and including the start
     time and the second is used at and after the end time.

     \gmXmlTag{gmGraphics,TransitionCoordinatesMapper,timeRange}
  */
  void setTimeRange(gmCore::float2 t);

  /**
     Set the start and end frame at which the transition should be
     made. The first mapper is used up until and including the start
     frame and the second is at and after the end frame.

     \gmXmlTag{gmGraphics,TransitionCoordinatesMapper,frameRange}
  */
  void setFrameRange(gmCore::size2 f);

  /**
     Set a static ratio of interpolation between the two
     mappers. Default is 0.5.

     \gmXmlTag{gmGraphics,TransitionCoordinatesMapper,ratio}
  */
  void setRatio(float n);

  /**
     Adds a coordinates mapper that should be used to map the 3D
     surrounding into the 2D view. Two are needed for transition.
  */
  void addCoordinatesMapper(std::shared_ptr<CoordinatesMapper> m);

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
