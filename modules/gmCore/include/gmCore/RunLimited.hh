
#ifndef GRAMODS_CORE_RUNLIMITED
#define GRAMODS_CORE_RUNLIMITED

#include <chrono>

#define GM_RUNLIMITED(CODE, SECONDS)                                    \
  {                                                                     \
    static std::chrono::steady_clock::time_point _gramods_last_executed; \
    std::chrono::steady_clock::time_point _gramods_now = std::chrono::steady_clock::now(); \
    if (_gramods_now - _gramods_last_executed > std::chrono::seconds(SECONDS)) { \
      _gramods_last_executed = _gramods_now;                            \
      CODE;                                                             \
    }                                                                   \
  }

#define GM_RUNLIMITED_BEGIN(SECONDS)            \
  {                                             \
  static bool _gramods_executed = false;        \
  if (!_gramods_executed) {                     \
  _gramods_executed = true;

#define GM_RUNLIMITED_END }}//

#endif
