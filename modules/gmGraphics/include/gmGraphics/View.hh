
#ifndef GRAMODS_GRAPHICS_VIEW
#define GRAMODS_GRAPHICS_VIEW

#include <gmGraphics/ViewBase.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The base of graphics View implementations. A view is a projection
   of rendered data into a buffer.
*/
class View : public ViewBase {

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  virtual std::string getDefaultKey() override { return "view"; }

public:

  GM_OFI_DECLARE;
};

END_NAMESPACE_GMGRAPHICS;

#endif
