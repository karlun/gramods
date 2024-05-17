
#ifndef GRAMODS_GRAPHICS_TRANSFORM
#define GRAMODS_GRAPHICS_TRANSFORM

#include <gmGraphics/Group.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The scenegraph transform base.
*/
class Transform : public Group {

public:
  virtual Eigen::Affine3f getTransform() = 0;
};

END_NAMESPACE_GMGRAPHICS;

#endif
