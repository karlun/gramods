
#ifndef GRAMODS_TRACK_VRPNSERVER
#define GRAMODS_TRACK_VRPNSERVER

#include <gmTrack/config.hh>

#ifdef gramods_ENABLE_VRPN

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>

#include <gmTrack/TrackerBase.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   This class sets up a VRPN server that post data from BinaryTracker,
   FloatTracker, Float2Tracker, and PoseTracker instances.

   The keys of the tracker states available from each tracker are used
   as VRPN tracker names and sensor indices, so for a key "wand/5" in
   a PoseTracker the VrpnServer will set up a VRPN pose remote named
   "wand" and report that pose as channel 5. VRPN does not provide a
   separate type for 2D data, so Float2Tracker must be explicitly
   structured. For keys that do not end with numbers, x and y are
   encoded into channels 0 and 1, respectively. For keys that do end
   with numbers, the x and y data are split up into separate analog
   server with keys appending "/x" and "/y", respecively, with
   channels determined by those numbers.

   This class configures as an Updateable with a priority of
   -100. Either Updateable::updateAll or update must be called at
   even intervals. This is done automatically by gm-load.
*/
class VrpnServer
  : public gmCore::Object {

public:

  VrpnServer();
  ~VrpnServer();

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
     Add a BinaryTracker to serve from this VRPN server.
  */
  void addBinaryTracker(std::shared_ptr<BinaryTracker> t);

  /**
     Add a FloatTracker to serve from this VRPN server.
  */
  void addFloatTracker(std::shared_ptr<FloatTracker> t);

  /**
     Add a Float2Tracker to serve from this VRPN server.
  */
  void addFloat2Tracker(std::shared_ptr<Float2Tracker> t);

  /**
     Add a PoseTracker to serve from this VRPN server.
  */
  void addPoseTracker(std::shared_ptr<PoseTracker> t);

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
