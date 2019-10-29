
#include <gmTypes/angle.hh>

#include <gmCore/MathConstants.hh>
#include <gmCore/Console.hh>

#include <sstream>
#include <cmath>

BEGIN_NAMESPACE_GMTYPES;

const float angle::from_degrees = (float)(GM_PI / 180.0);
const float angle::to_degrees = (float)(180.f / GM_PI);

END_NAMESPACE_GMTYPES;


BEGIN_NAMESPACE_GRAMODS;

std::istream& operator>> (std::istream &in, gmTypes::angle &v) {

  std::string str_angle;
  in >> str_angle;

  if (str_angle.empty())
    return in;

  int idx = 0;
  float multiplier = 1.f;
  if (str_angle[0] == 'd') {
    idx = 1;
    multiplier = gmTypes::angle::from_degrees;
  } else if (str_angle[0] == 'r') {
    idx = 1;
  }

  std::stringstream ss(str_angle.substr(idx));

  float value;
  ss >> value;

  if (!ss) {
    v = 0.f;
    in.setstate(std::ios_base::failbit);
    if (idx > 0)
      GM_WRN("operator>>(std::istream, str_angle)",
             "Cannot parse istream data '" << str_angle << "'"
             " (" << str_angle.substr(idx) << ") into str_angle.");
    else
      GM_WRN("operator>>(std::istream, str_angle)",
             "Cannot parse istream data '" << str_angle << "' into str_angle.");
  } else {
    v = multiplier * value;

    if (idx == 0 && fabsf(value) > 5 * GM_PI)
      GM_WRN("operator>>(std::istream, str_angle)",
             "Parsed str_angle (" << value << ") is large to be in radians"
             " - did yoy mean degrees ('d" << str_angle << "')? Otherwise"
             " use 'r" << str_angle << "' do disable this warning.");
  }

  return in;
}

END_NAMESPACE_GRAMODS;
