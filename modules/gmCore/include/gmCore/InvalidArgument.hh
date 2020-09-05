
#ifndef GRAMODS_CORE_INVALIDARGUMENT
#define GRAMODS_CORE_INVALIDARGUMENT

#include <gmCore/config.hh>

#include <string>

BEGIN_NAMESPACE_GMCORE;

/**
   Standard exception for invalid arguments in a call to a function or
   object. In well written software this exception is never thrown.
*/
struct InvalidArgument {

  /**
     Creates an exception with a clarification message.
  */
  InvalidArgument(std::string what)
    : what(what) {}

  /**
     A message with information about how preconditions were violated.
  */
  const std::string what;
};

END_NAMESPACE_GMCORE;

#endif
