
#include <gmTrack/Controller.hh>

BEGIN_NAMESPACE_GMTRACK;


GM_OFI_DEFINE(Controller);
GM_OFI_POINTER(Controller, poseTracker, SinglePoseTracker, Controller::setPoseTracker);
GM_OFI_POINTER(Controller, buttonsTracker, ButtonsTracker, Controller::setButtonsTracker);
GM_OFI_POINTER(Controller, analogsTracker, AnalogsTracker, Controller::setAnalogsTracker);


bool Controller::getPose(SinglePoseTracker::PoseSample &p) {
  if (!pose_tracker) return false;
  return pose_tracker->getPose(p);
}

bool Controller::getButtons(ButtonsTracker::ButtonsSample &b) {
  if (!buttons_tracker) return false;
  return buttons_tracker->getButtons(b);
}

bool Controller::getAnalogs(AnalogsTracker::AnalogsSample &a) {
  if (!analogs_tracker) return false;
  return analogs_tracker->getAnalogs(a);
}

void Controller::setPoseTracker(std::shared_ptr<SinglePoseTracker> spt) {
  pose_tracker = spt;
}

void Controller::setButtonsTracker(std::shared_ptr<ButtonsTracker> bt) {
  buttons_tracker = bt;
}

void Controller::setAnalogsTracker(std::shared_ptr<AnalogsTracker> at) {
  analogs_tracker = at;
}

END_NAMESPACE_GMTRACK;
