
#include <gmCore/io_string.hh>

#include <gmCore/Console.hh>

#include <sstream>

BEGIN_NAMESPACE_GRAMODS;

std::istream& operator>> (std::istream &in, gramods::gmCore::string2 &s) {

  std::string str;
  if (!(in >> str)) return in;

  s[0] = str;
  if (!str.empty() && str.front() == '"') {
    do {
      if (!(in >> str)) return in;
      s[0] += str;
    } while (!str.empty() && str.back() != '"');
  }

  if (!(in >> str)) return in;

  s[1] = str;
  if (!str.empty() && str.front() == '"') {
    do {
      if (!(in >> str)) return in;
      s[1] += str;
    } while (!str.empty() && str.back() != '"');
  }

  return in;
}

std::ostream& operator<< (std::ostream &out, gramods::gmCore::string2 &s) {
  out << s[0] << " " << s[1];
  return out;
}

END_NAMESPACE_GRAMODS;
