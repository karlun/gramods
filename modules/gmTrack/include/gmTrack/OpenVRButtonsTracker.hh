
#ifndef GRAMODS_TRACK_OPENVRBUTTONSTRACKER
#define GRAMODS_TRACK_OPENVRBUTTONSTRACKER

#include <gmTrack/ButtonsTracker.hh>

#ifdef gramods_ENABLE_OpenVR

#include <gmCore/OpenVR.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   Buttons tracker reading data off one or more actions in OpenVR.
*/
class OpenVRButtonsTracker : public ButtonsTracker {

public:
  OpenVRButtonsTracker();
  ~OpenVRButtonsTracker();

  /**
     Set the OpenVR instance.

     \gmXmlTag{gmTrack,OpenVRButtonsTracker,openVR}
  */
  void setOpenVR(std::shared_ptr<gmCore::OpenVR> openvr);

  /**
     Add an action to read out data from. The data from all added
     actions will be concatenated into the resulting sample.

     \gmXmlTag{gmTrack,OpenVRButtonsTracker,action}
  */
  void addAction(std::string name);

  /**
     Replaces the contents of p with analog data.
  */
  bool getButtons(ButtonsSample &b) override;

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

END_NAMESPACE_GMTRACK;

#endif

#endif
