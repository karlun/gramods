
#ifndef GRAMODS_TRACK_VRPNBUTTONSTRACKER
#define GRAMODS_TRACK_VRPNBUTTONSTRACKER

#include <gmTrack/ButtonsTracker.hh>

#ifdef gramods_ENABLE_VRPN

#include <vrpn_Button.h>

#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   Buttons tracker reading data off a VRPN server.
*/
class VRPNButtonsTracker
  : public ButtonsTracker {

public:

  VRPNButtonsTracker();
  ~VRPNButtonsTracker();

  /**
     The address to the button tracker at the VRPN server, such as
     "tracker@localhost".
  */
  void setConnectionString(std::string id);

  /**
     Replaces the contents of p with button data.
  */
  bool getButtons(ButtonsSample &p);

  GM_OFI_DECLARE(VRPNButtonsTracker);

private:

  std::unique_ptr<vrpn_Button_Remote> tracker;
	static void VRPN_CALLBACK handler(void *userdata, const vrpn_BUTTONCB info);

  ButtonsSample latest_sample;
  bool got_data;
};

END_NAMESPACE_GMTRACK;

#endif

#endif
