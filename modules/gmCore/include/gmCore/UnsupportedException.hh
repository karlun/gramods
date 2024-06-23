
#ifndef GRAMODS_CORE_UNSUPPORTEDEXCEPTION
#define GRAMODS_CORE_UNSUPPORTEDEXCEPTION

#include <gmCore/config.hh>

#include <string>

BEGIN_NAMESPACE_GMCORE;

/**
   Exception thrown when a feature is not supported.
*/
struct UnsupportedException {

  /**
     Creates an exception with a clarification message.
  */
  UnsupportedException(std::string what)
    : what(what) {}

  /**
     A message with information about what is unsupported and why.
  */
  const std::string what;
};

END_NAMESPACE_GMCORE;

#endif
