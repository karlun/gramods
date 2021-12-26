
#include <gmGraphics/Eye.hh>

#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

const Eye Eye::MONO = { 0, 1 };
const Eye Eye::LEFT = { 0, 2 };
const Eye Eye::RIGHT = { 1, 2 };

END_NAMESPACE_GMGRAPHICS;

BEGIN_NAMESPACE_GRAMODS;

std::istream& operator>> (std::istream &in, gmGraphics::Eye &e) {

  std::string str;
  in >> str;

  if (str.empty())
    return in;

  if (str == "MONO") {
    e = gmGraphics::Eye::MONO;
    return in;
  }

  if (str == "LEFT") {
    e = gmGraphics::Eye::LEFT;
    return in;
  }

  if (str == "RIGHT") {
    e = gmGraphics::Eye::RIGHT;
    return in;
  }

  in.setstate(std::ios_base::failbit);
  GM_WRN("operator>>(std::istream, gmGraphics::Eye&)",
             "Cannot parse istream data '" << str << "'"
             " as a valid Eye (MONO, LEFT, RIGHT).");

  return in;
}

END_NAMESPACE_GRAMODS;
