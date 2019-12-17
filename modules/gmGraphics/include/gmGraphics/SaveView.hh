
#ifndef GRAMODS_GRAPHICS_SAVEVIEW
#define GRAMODS_GRAPHICS_SAVEVIEW

#include <gmGraphics/View.hh>

#include <gmTypes/size.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The SaveView is a view that saves the image generated by another
   view.
*/
class SaveView
  : public View {

public:

  SaveView();

  /**
     Forwards rendering to the sub view and saves the result.
  */
  void renderFullPipeline(ViewSettings settings);

  /**
     Sets the file path to save the view to.

     If a sequence of image files should be saved, then set this as a
     fprint formatted template, for example "frame_%06d.png". Default
     value is "SaveView.png".

     Supported suffixes are png, jpeg, exr, tiff and pfm. Some of
     these support alpha and some support float:

     | Format | Alpha | Float |
     |:-------|:-----:|:-----:|
     | PNG    |   *   |       |
     | JPEG   |       |       |
     | EXR    |       |   *   |
     | TIFF   |   *   |   *   |
     | PFM    |       |   *   |

     \b XML-attribute: \c file
  */
  void setFile(std::string file);

  /**
     Sets the resolution to render and save at, regarless of
     resolution of the parent. Default is to use the parent.

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
  virtual void clearRenderers(bool recursive = false);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
