
#ifndef GRAMODS_CORE_INVALIDARGUMENT
#define GRAMODS_CORE_INVALIDARGUMENT

#include <gmCore/RuntimeException.hh>

#include <string>

BEGIN_NAMESPACE_GMCORE;

/**
   Standard exception for invalid arguments in a call to a function or
   object. In well written software this exception is never thrown.
*/
struct InvalidArgument : RuntimeException {

  /**
     Creates an exception with a clarification message.
  */
  InvalidArgument(std::string what)
    : RuntimeException(what) {}
};

END_NAMESPACE_GMCORE;

#endif
