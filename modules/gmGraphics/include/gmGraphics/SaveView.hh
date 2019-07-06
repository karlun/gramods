
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
     Sets the file path to save the view to. If a sequence of image
     files should be saved, then set this as a fprint formatted
     template, for example "frame_%06d.png". Default value is
     "SaveView.png". Supported suffixes are .png and .jpeg.

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
     Sets the view that should be saved to file.

     \b XML-key: \c view
  */
  void setView(std::shared_ptr<View> view);

  GM_OFI_DECLARE;

private:

  class Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
