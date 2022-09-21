
#include <gmGraphics/CoordinatesMapper.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

namespace {
size_t mappers_count = 0;
}

CoordinatesMapper::CoordinatesMapper()
  : var_id(GM_STR("CM" << mappers_count++)) {}
CoordinatesMapper::~CoordinatesMapper() {}

const std::string CoordinatesMapper::withVarId(std::string code) const {
  size_t idx;
  while ((idx = code.find("ID")) != std::string::npos)
    code.replace(idx, 2, var_id);
  return code;
}

END_NAMESPACE_GMGRAPHICS;
