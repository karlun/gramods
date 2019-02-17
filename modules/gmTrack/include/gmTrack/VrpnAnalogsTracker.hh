
#ifndef GRAMODS_TRACK_VRPNANALOGSTRACKER
#define GRAMODS_TRACK_VRPNANALOGSTRACKER

#include <gmTrack/AnalogsTracker.hh>

#ifdef gramods_ENABLE_VRPN

#include <vrpn_Analog.h>

#include <gmCore/OFactory.hh>
#include <gmCore/Updateable.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   Analogs tracker reading data off a VRPN server.

   This class configures as an Updateable with a priority of
   10. Either Updateable::updateAll or update must be called at even
   intervals. This is done automatically by gm-load.
*/
class VrpnAnalogsTracker
  : public AnalogsTracker,
    public gmCore::Updateable {

public:

  VrpnAnalogsTracker();
  ~VrpnAnalogsTracker();

  /**
     Updates the animation.
  */
  void update(gmCore::Updateable::clock::time_point t);

  /**
     The address to the button tracker at the VRPN server, such as
     "tracker@localhost".
  */
  void setConnectionString(std::string id);

  /**
     Replaces the contents of p with analog data.
  */
  bool getAnalogs(AnalogsSample &b);

  GM_OFI_DECLARE;

private:

  std::unique_ptr<vrpn_Analog_Remote> tracker;
	static void VRPN_CALLBACK handler(void *userdata, const vrpn_ANALOGCB info);

  AnalogsSample latest_sample;
  bool got_data;
  bool have_data = false;
};

END_NAMESPACE_GMTRACK;

#endif

#endif
