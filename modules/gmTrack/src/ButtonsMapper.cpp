
#include <gmTrack/ButtonsMapper.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(ButtonsMapper);
GM_OFI_PARAM(ButtonsMapper, mapping, gmTypes::size2, ButtonsMapper::addMapping);
GM_OFI_POINTER(ButtonsMapper, buttonsTracker, ButtonsTracker, ButtonsMapper::setButtonsTracker);

void ButtonsMapper::setButtonsTracker(std::shared_ptr<ButtonsTracker> bt) {
  buttonsTracker = bt;
}

void ButtonsMapper::addMapping(gmTypes::size2 m) {
  mappings[m[0]] = m[1];
}

bool ButtonsMapper::getButtons(ButtonsSample &p) {

  if (!buttonsTracker)
    return false;

  ButtonsSample p0;
  if (!buttonsTracker->getButtons(p0))
    return false;

  p.time = p0.time;
  p.buttons = 0;

  for (auto m : mappings)
    p.buttons |= ((p.buttons >> m.first) & 1) << m.second;

  return true;
}

END_NAMESPACE_GMTRACK;
