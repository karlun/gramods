
#ifndef GRAMODS_GRAPHICS_VIEW
#define GRAMODS_GRAPHICS_VIEW

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The base of graphics View implementations.
*/
class View
  : public gmCore::Object {

public:

  /**
     Asks the View to call its views for rendering.
   */
  void renderFullPipeline(Camera::RenderFunction *func = 0);

protected:
  
};

END_NAMESPACE_GMGRAPHICS;

#endif
