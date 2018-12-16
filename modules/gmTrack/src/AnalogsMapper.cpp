
#include <gmTrack/AnalogsMapper.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(AnalogsMapper);
GM_OFI_PARAM(AnalogsMapper, mapping, gmTypes::size2, AnalogsMapper::addMapping);
GM_OFI_PARAM(AnalogsMapper, verticalAnalog, int, AnalogsMapper::setVerticalAnalog);
GM_OFI_PARAM(AnalogsMapper, horizontalAnalog, int, AnalogsMapper::setHorizontalAnalog);
GM_OFI_PARAM(AnalogsMapper, triggerAnalog, int, AnalogsMapper::setTriggerAnalog);
GM_OFI_POINTER(AnalogsMapper, analogsTracker, AnalogsTracker, AnalogsMapper::setAnalogsTracker);

void AnalogsMapper::setAnalogsTracker(std::shared_ptr<AnalogsTracker> bt) {
  analogsTracker = bt;
}

void AnalogsMapper::addMapping(gmTypes::size2 m) {
  mappings[m[0]] = m[1];
}

void AnalogsMapper::setVerticalAnalog(int idx) {
  mappings[idx] = AnalogIndex::VERTICAL;
}

void AnalogsMapper::setHorizontalAnalog(int idx) {
  mappings[idx] = AnalogIndex::HORIZONTAL;
}

void AnalogsMapper::setTriggerAnalog(int idx) {
  mappings[idx] = AnalogIndex::TRIGGER;
}

bool AnalogsMapper::getAnalogs(AnalogsSample &p) {

  if (!analogsTracker)
    return false;

  AnalogsSample p0;
  if (!analogsTracker->getAnalogs(p0))
    return false;

  p.time = p0.time;
  p.analogs.clear();
  p.analogs.resize(AnalogIndex::COUNT, 0);

  for (auto m : mappings) {
    if (m.first >= p0.analogs.size())
      continue;
    if (m.second >= p.analogs.size())
      p.analogs.resize(m.second + 1, 0.f);
    p.analogs[m.second] = p0.analogs[m.first];
  }

  return true;
}

END_NAMESPACE_GMTRACK;
