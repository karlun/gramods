
#ifndef GRAMODS_TRACK_OPENVRFLOATTRACKER
#define GRAMODS_TRACK_OPENVRFLOATTRACKER

#include <gmTrack/TrackerBase.hh>

#ifdef gramods_ENABLE_OpenVR

#include <gmCore/OpenVR.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   Analogs tracker reading data off one or more keys in OpenVR.
*/
class OpenVRFloatTracker : public FloatTracker {

public:
  OpenVRFloatTracker();
  ~OpenVRFloatTracker();

  /**
     Set the OpenVR instance.

     \gmXmlTag{gmTrack,OpenVRFloatTracker,openVR}
  */
  void setOpenVR(std::shared_ptr<gmCore::OpenVR> openvr);

  /**
     Add an key to read out data from. The data from all added
     keys will be concatenated into the resulting sample.

     \gmXmlTag{gmTrack,OpenVRFloatTracker,key}
  */
  void addKey(std::string name);

  /**
     @see PoseTracker::get
  */
  std::optional<State> get() override;

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
