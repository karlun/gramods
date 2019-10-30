
#include <gmTrack/ButtonsMapper.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>


BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(ButtonsMapper);
GM_OFI_PARAM(ButtonsMapper, mapping, gmTypes::size2, ButtonsMapper::addMapping);
GM_OFI_PARAM(ButtonsMapper, mainButton, int, ButtonsMapper::setMainButton);
GM_OFI_PARAM(ButtonsMapper, secondaryButton, int, ButtonsMapper::setSecondaryButton);
GM_OFI_PARAM(ButtonsMapper, menuButton, int, ButtonsMapper::setMenuButton);
GM_OFI_POINTER(ButtonsMapper, buttonsTracker, ButtonsTracker, ButtonsMapper::setButtonsTracker);

void ButtonsMapper::setButtonsTracker(std::shared_ptr<ButtonsTracker> bt) {
  buttonsTracker = bt;
}

void ButtonsMapper::addMapping(gmTypes::size2 m) {
  mappings[m[0]] = m[1];
}

void ButtonsMapper::setMainButton(int idx) {
  mappings[idx] = 0;
}

void ButtonsMapper::setSecondaryButton(int idx) {
  mappings[idx] = 1;
}

void ButtonsMapper::setMenuButton(int idx) {
  mappings[idx] = 2;
}

bool ButtonsMapper::getButtons(ButtonsSample &p) {

  if (!buttonsTracker) {
    GM_RUNONCE(GM_WRN("ButtonsMapper", "Buttons requested by no buttons tracker available."));
    return false;
  }

  ButtonsSample p0;
  if (!buttonsTracker->getButtons(p0))
    return false;

  p.time = p0.time;
  p.buttons = 0;

  for (auto m : mappings)
    p.buttons |= ((p0.buttons >> m.first) & 1) << m.second;

  return true;
}

END_NAMESPACE_GMTRACK;
