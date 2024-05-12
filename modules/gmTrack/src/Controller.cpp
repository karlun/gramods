
#include <gmTrack/Controller.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>


BEGIN_NAMESPACE_GMTRACK;


GM_OFI_DEFINE(Controller);
GM_OFI_POINTER2(Controller, singlePoseTracker, SinglePoseTracker, setSinglePoseTracker);
GM_OFI_POINTER2(Controller, buttonsTracker, ButtonsTracker, setButtonsTracker);
GM_OFI_POINTER2(Controller, analogsTracker, AnalogsTracker, setAnalogsTracker);


bool Controller::getPose(SinglePoseTracker::PoseSample &p) {
  if (!single_pose_tracker) {
    GM_RUNONCE(GM_WRN("Controller", "Pose requested but no pose tracker available."));
    return false;
  }
  return single_pose_tracker->getPose(p);
}

bool Controller::getButtons(ButtonsTracker::ButtonsSample &b) {
  if (!buttons_tracker) {
    GM_RUNONCE(GM_WRN("Controller", "Buttons requested but no buttons tracker available."));
    return false;
  }
  return buttons_tracker->getButtons(b);
}

bool Controller::getAnalogs(AnalogsTracker::AnalogsSample &a) {
  if (!analogs_tracker) {
    GM_RUNONCE(GM_WRN("Controller", "Analogs requested but no analogs tracker available."));
    return false;
  }
  return analogs_tracker->getAnalogs(a);
}

void Controller::setSinglePoseTracker(std::shared_ptr<SinglePoseTracker> spt) {
  single_pose_tracker = spt;
}

void Controller::setButtonsTracker(std::shared_ptr<ButtonsTracker> bt) {
  buttons_tracker = bt;
}

void Controller::setAnalogsTracker(std::shared_ptr<AnalogsTracker> at) {
  analogs_tracker = at;
}

void Controller::traverse(Visitor *visitor) {
  if (single_pose_tracker) single_pose_tracker->accept(visitor);
  if (buttons_tracker) buttons_tracker->accept(visitor);
  if (analogs_tracker) analogs_tracker->accept(visitor);
}

END_NAMESPACE_GMTRACK;
