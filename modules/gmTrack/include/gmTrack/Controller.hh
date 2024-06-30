
#ifndef GRAMODS_TRACK_CONTROLLER
#define GRAMODS_TRACK_CONTROLLER

#include <gmTrack/config.hh>
#include <gmTrack/SinglePoseTracker.hh>
#include <gmTrack/ButtonsTracker.hh>
#include <gmTrack/AnalogsTracker.hh>

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   A controller is a combination of one or more of a pose tracker,
   buttons tracker and analogs tracker, also called Wand.

   This class does not do anything but group trackers together and
   forward call to them.

   Observe that a stationary controller may not have a pose tracker
   associated with it and calling getPose would then return
   false. Likewise one or more of the other trackers may also be
   missing in a valid configuration.
*/
class Controller
  : public gmCore::Object {

public:

  /**
     Replaces the contents of p with pose data. Returns true on
     success, false otherwise.
  */
  bool getPose(PoseTracker::PoseSample &p);

  /**
     Replaces the contents of p with buttons data. Returns true on
     success, false otherwise.
  */
  bool getButtons(ButtonsTracker::ButtonsSample &b);

  /**
     Replaces the contents of p with analogs data. Returns true on
     success, false otherwise.
  */
  bool getAnalogs(AnalogsTracker::AnalogsSample &a);

  /**
     Sets the pose tracker associated with this controller.

     \gmXmlTag{gmTrack,Controller,singlePoseTracker}
  */
  void setSinglePoseTracker(std::shared_ptr<SinglePoseTracker> spt);

  /**
     Sets the buttons tracker associated with this controller. This is
     associated with the attribute buttonsTracker in configuration
     data.

     \gmXmlTag{gmTrack,Controller,buttonsTracker}
  */
  void setButtonsTracker(std::shared_ptr<ButtonsTracker> bt);

  /**
     Sets the analogs tracker associated with this controller. This is
     associated with the attribute analogsTracker in configuration
     data.

     \gmXmlTag{gmTrack,Controller,analogsTracker}
  */
  void setAnalogsTracker(std::shared_ptr<AnalogsTracker> at);

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  virtual std::string getDefaultKey() override { return "controller"; }

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  GM_OFI_DECLARE;

private:

  std::shared_ptr<SinglePoseTracker> single_pose_tracker;

  std::shared_ptr<ButtonsTracker> buttons_tracker;

  std::shared_ptr<AnalogsTracker> analogs_tracker;
};

END_NAMESPACE_GMTRACK;

#endif
