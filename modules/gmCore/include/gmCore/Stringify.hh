
#ifndef GRAMODS_CORE_STRINGIFY
#define GRAMODS_CORE_STRINGIFY

#include <sstream>
#include <vector>
#include <map>

#define GM_STR(X) (static_cast<std::stringstream&>(std::stringstream().flush() << X).str())

BEGIN_NAMESPACE_GMCORE;

template<class T>
std::string stringify(std::vector<T> list) {

  std::stringstream ss;
  bool is_first = true;

  for (auto &item : list) {
    if (!is_first) ss << ", ";
    else is_first = false;
    ss << item;
  }

  return ss.str();
}

template<class T1, class T2>
std::string stringify(std::map<T1, T2> list, bool print_first = true) {

  std::stringstream ss;
  bool is_first = true;

  if (print_first)
    for (auto &item : list) {
      if (!is_first) ss << ", ";
      else is_first = false;
      ss << item.first;
    }
  else
    for (auto &item : list) {
      if (!is_first) ss << ", ";
      else is_first = false;
      ss << item.second;
    }

  return ss.str();
}

END_NAMESPACE_GMCORE;

#endif
