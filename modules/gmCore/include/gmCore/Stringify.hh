
#ifndef GRAMODS_CORE_STRINGIFY
#define GRAMODS_CORE_STRINGIFY

#define GM_STR(X) (static_cast<std::stringstream&>(std::stringstream() << X).str())

#endif
