
#ifndef GRAMODS_TRACK_OPENVRPOSETRACKER
#define GRAMODS_TRACK_OPENVRPOSETRACKER

#include <gmTrack/TrackerBase.hh>

#ifdef gramods_ENABLE_OpenVR

#include <gmCore/OpenVR.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   The base of OpenVRPoseTrackers
 */
class OpenVRPoseTracker : public PoseTracker {

public:
  OpenVRPoseTracker();
  ~OpenVRPoseTracker();

  /**
     Adds a device type (class) to track. When the specified type, role
     and/or serial number does not match an available tracker, no data
     will be returned.

     \gmXmlTag{gmTrack,OpenVRPoseTracker,type}
  */
  void addType(std::string);

  /**
     Adds a device role to track. When the specified type, role
     and/or serial number does not match an available tracker, no data
     will be returned.

     \gmXmlTag{gmTrack,OpenVRPoseTracker,role}
  */
  void addRole(std::string);

  /**
     Adds a serial number of the device to track. When the specified
     type, role and/or serial number does not match an available
     tracker, no data will be returned.

     \gmXmlTag{gmTrack,OpenVRPoseTracker,serial}
  */
  void addSerial(std::string);

  /**
     @see PoseTracker::get
  */
  std::optional<State> get() override;

  /**
     Set the OpenVR instance.

     \gmXmlTag{gmTrack,OpenVRPoseTracker,openVR}
  */
  void setOpenVR(std::shared_ptr<gmCore::OpenVR> openvr);

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
