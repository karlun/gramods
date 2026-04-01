
#ifndef GRAMODS_CORE_JSON
#define GRAMODS_CORE_JSON

#include <gmCore/config.hh>

#ifdef gramods_ENABLE_nlohmann_json

#include <nlohmann/json.hpp>

#include <iostream>

BEGIN_NAMESPACE_GRAMODS;

/// Stream operator reading nlohmann::json
inline std::istream& operator>> (std::istream &in, nlohmann::json &j) {
  std::string s;
  if (in >> s) j = nlohmann::json::parse(s);
  return in;
}

/// Stream operator writing nlohmann::json
inline std::ostream& operator<< (std::ostream &out, const nlohmann::json &j) {
  out << j.dump();
  return out;
}

END_NAMESPACE_GRAMODS;

#endif
#endif
