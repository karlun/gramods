
#ifndef GRAMODS_CORE_RUNONCE
#define GRAMODS_CORE_RUNONCE

/** \def GM_RUNONCE(CODE)
   Macro for executing a small piece of code only once, typically a
   warning or error message command.

   @param CODE The code to execute once.
*/

/** \def GM_RUNONCE_BEGIN
   Macro creating a block that will be executed only once, typically a
   warning or error message command. Must be ended with GM_RUNONCE_END.
*/

#define GM_RUNONCE(CODE)                        \
  {                                             \
  static bool _gramods_executed = false;        \
  if (!_gramods_executed) {                     \
  _gramods_executed = true;                     \
  CODE;                                         \
  }                                             \
  }

#define GM_RUNONCE_BEGIN                        \
  {                                             \
  static bool _gramods_executed = false;        \
  if (!_gramods_executed) {                     \
  _gramods_executed = true;

#define GM_RUNONCE_END }}//

#endif
