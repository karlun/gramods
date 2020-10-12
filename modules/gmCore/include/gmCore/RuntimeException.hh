
#ifndef GRAMODS_CORE_RUNTIMEEXCEPTION
#define GRAMODS_CORE_RUNTIMEEXCEPTION

#include <gmCore/config.hh>

#include <string>

BEGIN_NAMESPACE_GMCORE;

/**
   Standard exception for runtime problems and base type for more
   detailed problems.
*/
struct RuntimeException {

  /**
     Creates an exception with a clarification message.
  */
  RuntimeException(std::string what)
    : what(what) {}

  /**
     A message with information about how preconditions were violated.
  */
  const std::string what;
};

END_NAMESPACE_GMCORE;

#endif
