
#ifndef GRAMODS_CORE_RUNONCE
#define GRAMODS_CORE_RUNONCE

#define GM_RUNONCE(XXX)                         \
  {                                             \
  static bool _gramods_executed = false;        \
  if (!_gramods_executed) {                     \
  _gramods_executed = true;                     \
  XXX;                                          \
  }                                             \
  }

#define GM_RUNONCE_BEGIN                        \
  {                                             \
  static bool _gramods_executed = false;        \
  if (!_gramods_executed) {                     \
  _gramods_executed = true;

#define GM_RUNONCE_END }}//

#endif
