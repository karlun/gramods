
#ifndef GRAMODS_TRACK_VRPNSERVER
#define GRAMODS_TRACK_VRPNSERVER

#include <gmTrack/config.hh>

#ifdef gramods_ENABLE_VRPN

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>
#include <gmCore/Updateable.hh>

#include <gmTrack/AnalogsTracker.hh>
#include <gmTrack/ButtonsTracker.hh>
#include <gmTrack/MultiPoseTracker.hh>
#include <gmTrack/SinglePoseTracker.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   This class sets up a VRPN server that post data from
   AnalogsTracker, ButtonsTracker, SinglePoseTracker and
   MultiPoseTracker instances.

   This class configures as an Updateable with a priority of
   -100. Either Updateable::updateAll or update must be called at
   even intervals. This is done automatically by gm-load.
*/
class VrpnServer
  : public gmCore::Object,
    public gmCore::Updateable {

public:

  VrpnServer();
  ~VrpnServer();

  /**
     Reads off analogs, buttons and pose data, and send these through
     the VRPN server.
  */
  void update(gmCore::Updateable::clock::time_point time,
              size_t frame) override;

  /**
     Sets up the server. This should be called once only!
  */
  void initialize() override;

  /**
     Sets the port to bind the VRPN server to.

     \gmXmlTag{gmTrack,VrpnServer,port}
  */
  void setPort(int port);

  /**
     Add a name to associate with a tracker in this VRPN server. There
     must be exactly one name for each tracker served.
  */
  void addTrackerName(std::string name);

  /**
     Add an AnalogsTracker to serve from this VRPN server. There must
     be exactly one name for each tracker served.
  */
  void addAnalogsTracker(std::shared_ptr<AnalogsTracker> t);

  /**
     Add a ButtonsTracker to serve from this VRPN server. There must
     be exactly one name for each tracker served.
  */
  void addButtonsTracker(std::shared_ptr<ButtonsTracker> t);

  /**
     Add a MultiPoseTracker to serve from this VRPN server. There must be
     exactly one name for each tracker served.
  */
  void addMultiPoseTracker(std::shared_ptr<MultiPoseTracker> t);

  /**
     Add a SinglePoseTracker to serve from this VRPN server. There must be
     exactly one name for each tracker served.
  */
  void addSinglePoseTracker(std::shared_ptr<SinglePoseTracker> t);

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
