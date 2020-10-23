
#ifndef GRAMODS_CORE_STRINGIFY
#define GRAMODS_CORE_STRINGIFY

#include <sstream>

#define GM_STR(X) (static_cast<std::stringstream&>(std::stringstream().flush() << X).str())

#endif
