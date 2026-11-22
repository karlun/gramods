
#ifndef GRAMODS_GRAPHICS_HMDVIEW
#define GRAMODS_GRAPHICS_HMDVIEW

#include <gmGraphics/config.hh>

#ifdef gramods_ENABLE_OpenVR

#include <gmCore/OpenVR.hh>

#include <gmGraphics/View.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   View that reads off head tracking and render a 3D scene to a HMD
   via OpenVR, and also displays the results in the current view.
*/
class HmdView
  : public View {

public:
  HmdView();

  /**
     Forwards rendering to the sub view, and shows the result both on
     the current OpenVR context and the view's buffer.
  */
  void renderFullPipeline(ViewSettings settings) override;

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  /**
     Sets if left eye (0, default), right eye (1) or both eyes (2)
     should be shown in the view.
  */
  void setShowEye(size_t t);

  /**
     Set the OpenVR instance.

     \gmXmlTag{gmGraphics,HmdView,openVR}
  */
  void setOpenVR(std::shared_ptr<gmCore::OpenVR> openvr);

  GM_OFI_DECLARE;

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
#endif
