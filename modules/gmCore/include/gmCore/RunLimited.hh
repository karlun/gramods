
#ifndef GRAMODS_CORE_RUNLIMITED
#define GRAMODS_CORE_RUNLIMITED

#include <chrono>

/**\def GM_RUNLIMITED(CODE, SECONDS)
   Macro for executing a small piece of code less frequently, limited
   to a specified number of seconds before executing again, typically
   a warning or error message command.

   @param CODE The code to execute more seldom.

   @param SECOND The number of seconds to wait (at least) before
   executing the code again.
*/

/**\def GM_RUNLIMITED_BEGIN(SECONDS)
   Macro creating a block that will be executed less frequencly,
   limited to a specified number of seconds before executing again,
   typically a warning or error message command. Must be ended with
   GM_RULIMITED_END.

   @param SECOND The number of seconds to wait (at least) before
   executing the code in this block again.
*/

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
