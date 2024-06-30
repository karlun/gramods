
#include <gmTrack/ButtonsMapper.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>


BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(ButtonsMapper);
GM_OFI_PARAM2(ButtonsMapper, mapping, gmCore::size2, addMapping);
GM_OFI_PARAM2(ButtonsMapper, mainButton, int, setMainButton);
GM_OFI_PARAM2(ButtonsMapper, secondaryButton, int, setSecondaryButton);
GM_OFI_PARAM2(ButtonsMapper, menuButton, int, setMenuButton);
GM_OFI_POINTER2(ButtonsMapper, buttonsTracker, ButtonsTracker, setButtonsTracker);

const size_t ButtonsMapper::ButtonIdx::MAIN      = 0;
const size_t ButtonsMapper::ButtonIdx::SECONDARY = 1;
const size_t ButtonsMapper::ButtonIdx::MENU      = 2;

void ButtonsMapper::setButtonsTracker(std::shared_ptr<ButtonsTracker> bt) {
  buttonsTracker = bt;
}

void ButtonsMapper::addMapping(gmCore::size2 m) {
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
    GM_RUNONCE(GM_WRN("ButtonsMapper", "Buttons requested but no buttons tracker available."));
    return false;
  }

  ButtonsSample p0;
  if (!buttonsTracker->getButtons(p0))
    return false;

  p.time = p0.time;
  p.buttons.clear();

  for (auto btn : p0.buttons)
    if (mappings.count(btn.first))
      p.buttons[mappings[btn.first]] = btn.second;
    else
      p.buttons[btn.first] = btn.second;

  return true;
}

void ButtonsMapper::traverse(Visitor *visitor) {
  if (buttonsTracker) buttonsTracker->accept(visitor);
}

END_NAMESPACE_GMTRACK;
