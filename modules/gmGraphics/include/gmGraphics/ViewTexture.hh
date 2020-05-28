
#ifndef GRAMODS_GRAPHICS_VIEWTEXTURE
#define GRAMODS_GRAPHICS_VIEWTEXTURE

#include <gmGraphics/View.hh>
#include <gmGraphics/TextureInterface.hh>

#include <gmTypes/size.hh>

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
     Sets the resolution to render and save at, regarless of
     resolution of the parent. Default is 1024x1024.

     \b XML-attribute: \c resolution
  */
  void setResolution(gmTypes::size2 res);

  /**
     Set to true if the pixel data should be read off and saved in
     floating point format. Default is off.
  */
  void setUseFloat(bool on);

  /**
     Returns true iff the pixel data are read off and saved in
     floating point format, false otherwise.
  */
  bool getUseFloat();

  /**
     Set to true if the alpha channel should be read off and
     saved. Default is false.
  */
  void setUseAlpha(bool on);

  /**
     Returns true iff the alpha channel is read off and saved.
  */
  bool getUseAlpha();

  /**
     Sets the view that should be saved to file.

     \b XML-key: \c view
  */
  void setView(std::shared_ptr<View> view);

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
