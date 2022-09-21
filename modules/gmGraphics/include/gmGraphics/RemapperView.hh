
#ifndef GRAMODS_GRAPHICS_REMAPPERVIEW
#define GRAMODS_GRAPHICS_REMAPPERVIEW

#include <gmGraphics/View.hh>
#include <gmGraphics/CoordinatesMapper.hh>

#include <gmCore/float.hh>
#include <gmCore/angle.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The RemapperView remaps another view from the mapping expressed by
   one CoordinatesMapper into the mapping expressed by another, in a
   single step.
*/
class RemapperView : public View {

public:
  RemapperView();
  virtual ~RemapperView();

  /**
     Appends local renderers to the view settings, forwards rendering
     to the sub views and mixes the results.
  */
  void renderFullPipeline(ViewSettings settings);

  /**
     Adds a view to mask with chroma key.

     \gmXmlTag{gmGraphics,RemapperView,view}
  */
  void addView(std::shared_ptr<View>);

  /**
     Sets the mapper to remap from, describing the mapping of the
     source, the input.

     \gmXmlTag{gmGraphics,RemapperView,from}
  */
  void setFrom(std::shared_ptr<CoordinatesMapper>);

  /**
     Sets the mapper to remap to, describing the mapping of the
     target, the result.

     \gmXmlTag{gmGraphics,RemapperView,to}
  */
  void setTo(std::shared_ptr<CoordinatesMapper>);

  /**
     Sets an offset for the view position. This component does not
     make intersection or 3D rendering, so this number will be used
     together with sphereRadius to calculate a sphere
     orientation. Default is 0.0;

     \gmXmlTag{gmGraphics,RemapperView,viewOffset}
  */
  void setViewOffset(float);

  /**
     Sets a sphere radius for the remapping. This component does not
     make intersection or 3D rendering, so this number will be used
     together with viewOffset to calculate a sphere
     orientation. Default is 1.0.

     \gmXmlTag{gmGraphics,RemapperView,sphereRadius}
  */
  void setSphereRadius(float);

  /**
     Sets an orientation offset for the remapping.

     \gmXmlTag{gmGraphics,RemapperView,orientation}
  */
  void setOrientation(gmCore::angle2);

  /**
     Removes all renderers and, if recursive is set to true, also
     renderers added to sub dispatchers.
  */
  virtual void clearRenderers(bool recursive = false) override;

  GM_OFI_DECLARE;

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
