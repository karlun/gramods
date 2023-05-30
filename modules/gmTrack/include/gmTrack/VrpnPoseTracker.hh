
#ifndef GRAMODS_TRACK_VRPNPOSETRACKER
#define GRAMODS_TRACK_VRPNPOSETRACKER

#include <gmTrack/MultiPoseTracker.hh>

#ifdef gramods_ENABLE_VRPN

#include <gmCore/OFactory.hh>
#include <gmCore/Updateable.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   Pose tracker reading data off a VRPN server.

   This class configures as an Updateable with a priority of
   10. Either Updateable::updateAll or update must be called at even
   intervals. This is done automatically by gm-load.
*/
class VrpnPoseTracker
  : public MultiPoseTracker,
    public gmCore::Updateable {

public:

  VrpnPoseTracker();
  ~VrpnPoseTracker();

  /**
     Updates the animation.
  */
  void update(gmCore::Updateable::clock::time_point time,
              size_t frame) override;

  /**
     The address to the pose tracker at the VRPN server, such as
     "tracker@localhost".

     \gmXmlTag{gmTrack,VrpnPoseTracker,connectionString}
  */
  void setConnectionString(std::string id);

  /**
     Replaces the contents of p with pose data.
  */
  bool getPose(std::map<int, PoseSample> &p) override;

  GM_OFI_DECLARE;

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMTRACK;

#endif

#endif
