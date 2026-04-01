
#ifndef GRAMODS_TRACK_VRPNTRACKER
#define GRAMODS_TRACK_VRPNTRACKER

#include <gmTrack/TrackerBase.hh>

#ifdef gramods_ENABLE_VRPN

#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   Tracker reading data off a VRPN server.

   The name part of the connection string is used as key string,
   with the sensor index appended at the end. For example, with
   `connectionString="phasespace/device@127.0.0.1"` you may get the
   following keys:

   - /phasespace/device/0
   - /phasespace/device/1
   - ...

   This class configures as an Updateable with a priority of
   10. Updateable::updateAll must be called at even intervals. This is
   done automatically by gm-load.
*/
template<class TYPE, class vrpn_TRACKER, class vrpn_CB>
class VrpnTracker : public TrackerBase<TYPE> {

protected:
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  VrpnTracker();
  virtual ~VrpnTracker();

  /**
     Adds an address to a pose tracker at a VRPN server, such as
     "tracker@localhost".

     \gmXmlNodeAttr{gmTrack,PoseVrpnTracker,connectionString}
     \gmXmlNodeAttr{gmTrack,FloatVrpnTracker,connectionString}
     \gmXmlNodeAttr{gmTrack,Float2VrpnTracker,connectionString}
     \gmXmlNodeAttr{gmTrack,BinaryVrpnTracker,connectionString}
  */
  void addConnectionString(std::string id);

  /**
     @see TrackerBase::get
  */
  std::optional<typename TrackerBase<TYPE>::State> get() override;
};

END_NAMESPACE_GMTRACK;

#endif
#endif
