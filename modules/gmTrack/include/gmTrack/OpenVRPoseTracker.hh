
#ifndef GRAMODS_TRACK_OPENVRPOSETRACKER
#define GRAMODS_TRACK_OPENVRPOSETRACKER

#include <gmTrack/SinglePoseTracker.hh>

#ifdef gramods_ENABLE_OpenVR

#include <gmCore/OpenVR.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   The base of OpenVRPoseTrackers
 */
class OpenVRPoseTracker
  : public SinglePoseTracker {

public:
  OpenVRPoseTracker();
  ~OpenVRPoseTracker();

  /**
     Set the device type (class) to track. When the specified type, role
     and/or serial number does not match an available tracker, no data
     will be returned.

     \gmXmlTag{gmTrack,OpenVRPoseTracker,type}
  */
  void setType(std::string);

  /**
     Set the device role to track. When the specified type, role
     and/or serial number does not match an available tracker, no data
     will be returned.

     \gmXmlTag{gmTrack,OpenVRPoseTracker,role}
  */
  void setRole(std::string);

  /**
     Set the serial number of the device to track. When the specified
     type, role and/or serial number does not match an available
     tracker, no data will be returned.

     \gmXmlTag{gmTrack,OpenVRPoseTracker,serial}
  */
  void setSerial(std::string);

  /**
     Replaces the contents of p with pose data from the specified
     OpenVR pose source. Returns true if data could be read, false
     otherwise. Use sample time to check that the data are fresh.
  */
  bool getPose(PoseSample &p);

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
