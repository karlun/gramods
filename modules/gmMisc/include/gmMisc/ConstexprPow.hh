
#ifndef GRAMODS_MISC_CONSTEXPRPOW
#define GRAMODS_MISC_CONSTEXPRPOW

#include <gmMisc/config.hh>

BEGIN_NAMESPACE_GMMISC;

template<typename T> constexpr T ipow(T num, unsigned int pow) {
  return pow == 0 ? 1 : num * ipow(num, pow - 1);
}

END_NAMESPACE_GMMISC;

#endif
