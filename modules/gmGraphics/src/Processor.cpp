
#include <gmGraphics/Processor.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

bool Processor::render(std::vector<globjects::Texture> projs) {
  if (cameras.empty()) return false;

  proj.clear();
  for (auto cam : cameras) {
    globjects::Texture tex;
    render(cam, tex);
    projs.push_back(tex);
  }

  return true;
}

END_NAMESPACE_GMGRAPHICS;

