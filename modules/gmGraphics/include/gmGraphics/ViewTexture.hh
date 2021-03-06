
#ifndef GRAMODS_GRAPHICS_VIEWTEXTURE
#define GRAMODS_GRAPHICS_VIEWTEXTURE

#include <gmGraphics/View.hh>
#include <gmGraphics/TextureInterface.hh>

#include <gmCore/size.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The ViewTexture is a View implements the TextureInterface to
   provide a texture of what is drawn by another view.

   As with any other hierarchy of views or renderers, cyclic graph are
   not allowed, e.g. to let a ViewTexture create a texture of the view
   containing the renderer using this texture.
*/
class ViewTexture
  : public View,
    public TextureInterface {

public:

  ViewTexture();

  /**
     Forwards rendering to the sub view and saves the result.
  */
  void renderFullPipeline(ViewSettings settings);

  /**
     Sets the resolution to render sub views at, regarless of
     resolution of the parent. Default is 1024x1024.

     \gmXmlTag{gmGraphics,ViewTexture,resolution}
  */
  void setResolution(gmCore::size2 res);

  /**
     Set to true if the texture should be floating point
     format. Default is off.
  */
  void setUseFloat(bool on);

  /**
     Returns true iff the texture is in floating point format, false
     otherwise.
  */
  bool getUseFloat();

  /**
     Set to true if the texture should have an alpha channel. Default
     is true.
  */
  void setUseAlpha(bool on);

  /**
     Returns true iff the texture should have an alpha channel.
  */
  bool getUseAlpha();

  /**
     Adds a view to render to this texture.

     \gmXmlTag{gmGraphics,ViewTexture,view}
  */
  void addView(std::shared_ptr<View> view);

  /**
     Removes all renderers and, if recursive is set to true, also
     renderers added to sub dispatchers.
  */
  void clearRenderers(bool recursive = false);

  /**
     Updates the texture with new data. Must be called with GL
     context.
  */
  void update();

  /**
     Returns the ID of the associated GL texture object.
  */
  GLuint getGLTextureID();

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  virtual std::string getDefaultKey() { return "texture"; }

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
