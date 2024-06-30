
#ifndef GRAMODS_TRACK_OPENVRANALOGSTRACKER
#define GRAMODS_TRACK_OPENVRANALOGSTRACKER

#include <gmTrack/AnalogsTracker.hh>

#ifdef gramods_ENABLE_OpenVR

#include <gmCore/OpenVR.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   Analogs tracker reading data off one or more actions in OpenVR.
*/
class OpenVRAnalogsTracker : public AnalogsTracker {

public:
  OpenVRAnalogsTracker();
  ~OpenVRAnalogsTracker();

  /**
     Set the OpenVR instance.

     \gmXmlTag{gmTrack,OpenVRAnalogsTracker,openVR}
  */
  void setOpenVR(std::shared_ptr<gmCore::OpenVR> openvr);

  /**
     Add an action to read out data from. The data from all added
     actions will be concatenated into the resulting sample.

     \gmXmlTag{gmTrack,OpenVRAnalogsTracker,action}
  */
  void addAction(std::string name);

  /**
     Replaces the contents of p with analog data.
  */
  bool getAnalogs(AnalogsSample &b) override;

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
