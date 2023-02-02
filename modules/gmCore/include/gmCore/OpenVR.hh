
#ifndef GRAMODS_CORE_OPENVR
#define GRAMODS_CORE_OPENVR

#include <gmCore/config.hh>

#ifdef gramods_ENABLE_OpenVR

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>
#include <gmCore/Updateable.hh>

#include <openvr.h>

#include <Eigen/Eigen>

#include <memory>
#include <array>
#include <optional>

BEGIN_NAMESPACE_GMCORE;

/**
   Initializer for OpenVR. Create one and share this between all
   components that require OpenVR. OpenVR will be initialized upon
   calling OpenVR::initialize (automatically called by Configuration)
   and deinitialized when all shared pointers are released and the
   object destroyed.
*/
class OpenVR : public gmCore::Object {

public:

  /**
     Interface for event consumers that can be registered with the
     OpenVR instance.
  */
  struct EventConsumer {

    /**
       Process an incoming OpenVR event and return true if the event
       was acted upon.

       @param event The OpenVR event to process.

       @param is_consumed Set to true if another event consumer has
       acted upon the event.

       @return True if the event has been acted upon, false otherwise.
    */
    virtual bool processEvent(const vr::VREvent_t *event, bool is_consumed) = 0;
  };

  typedef std::array<vr::TrackedDevicePose_t, vr::k_unMaxTrackedDeviceCount>
      pose_list_t;

  OpenVR();
  ~OpenVR();

  /**
     Enable or disable initialization of HMD composer. Default is
     false.

     \gmXmlTag{gmCore,OpenVR,useVideo}
  */
  void setUseVideo(bool on);

  /**
     Set path to manifest file, necessary for analog and button input.

     \gmXmlTag{gmCore,OpenVR,manifestPath}
  */
  void setManifestPath(std::filesystem::path);

  /**
     Set which action set to read out data from.

     \gmXmlTag{gmCore,OpenVR,actionSet}
  */
  void setActionSet(std::string name);

  void initialize() override;

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  std::string getDefaultKey() override { return "openVR"; }

  /**
     Retrieve immediate tracking data as a post list, without
     prediction. This is cached so multiple calls during one frame
     does not equate multiple calls to OpenVR.
  */
  std::optional<pose_list_t> &getPoseList();

  /**
     Add an object to receive events from OpenVR. This has no effect
     if the pointer is null or invalid or if the object has already
     been added. The event consumers will be called in the order the
     were added.
  */
  void addEventConsumer(std::weak_ptr<EventConsumer> ec);

  /**
     Remove an object to receive events from OpenVR. This has no
     effect if the pointer is null or if the object has not been
     added.
  */
  void removeEventConsumer(EventConsumer *ec);

  /**
     Returns the last time this instance was updated.
  */
  Updateable::clock::time_point getLastTime();

  /**
     Returns the analog data associated with the specified action.
  */
  std::optional<vr::InputAnalogActionData_t>
  getAnalog(std::string action,
            vr::VRInputValueHandle_t device = vr::k_ulInvalidInputValueHandle);

  /**
     Returns the digital data associated with the specified action.
  */
  std::optional<vr::InputDigitalActionData_t>
  getDigital(std::string action,
             vr::VRInputValueHandle_t device = vr::k_ulInvalidInputValueHandle);

  /**
     Retrieves the device type (class) of the tracked device with the
     specified index.
   */
  vr::ETrackedDeviceClass getType(size_t);

  /**
     Retrieves the role of the tracked device with the specified
     index.
  */
  vr::ETrackedControllerRole getRole(size_t);

  /**
     Retrieves the serial number of the tracked device with the
     specified index.
   */
  std::string getSerial(size_t);

  /**
     Retrieve a pointer to the OpenVR system.
  */
  vr::IVRSystem *ptr();

  /**
     Convert from OpenVR matrix format to Eigen 4x4 matrix.
  */
  static Eigen::Matrix4f convert(const vr::HmdMatrix34_t &m);

  /**
     Convert device type (class) to string, e.g. for print-out.
  */
  static std::string typeToString(vr::ETrackedDeviceClass type);

  /**
     Convert device role to string, e.g. for print-out.
  */
  static std::string roleToString(vr::ETrackedControllerRole role);

  GM_OFI_DECLARE;

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMCORE;

#endif
#endif
