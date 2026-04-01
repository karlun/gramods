
#ifndef GRAMODS_TRACK_OPENVRBINARYTRACKER
#define GRAMODS_TRACK_OPENVRBINARYTRACKER

#include <gmTrack/TrackerBase.hh>

#ifdef gramods_ENABLE_OpenVR

#include <gmCore/OpenVR.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   Buttons tracker reading data off one or more keys in OpenVR.
*/
class OpenVRBinaryTracker : public BinaryTracker {

public:
  OpenVRBinaryTracker();
  ~OpenVRBinaryTracker();

  /**
     Set the OpenVR instance.

     \gmXmlTag{gmTrack,OpenVRBinaryTracker,openVR}
  */
  void setOpenVR(std::shared_ptr<gmCore::OpenVR> openvr);

  /**
     Add a key to read out data from. The data from all added
     keys will be concatenated into the resulting sample.

     \gmXmlTag{gmTrack,OpenVRBinaryTracker,key}
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
