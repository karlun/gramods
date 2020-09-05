
#ifndef GRAMODS_CORE_PRECONDITIONVIOLATION
#define GRAMODS_CORE_PRECONDITIONVIOLATION

#include <gmCore/config.hh>

#include <string>

BEGIN_NAMESPACE_GMCORE;

/**
   Standard exception for violation of pre conditions in a call to a
   function or object. In well written software this exception is
   never thrown.
*/
struct PreConditionViolation {

  /**
     Creates an exception with a clarification message.
  */
  PreConditionViolation(std::string what)
    : what(what) {}

  /**
     A message with information about how preconditions were violated.
  */
  const std::string what;
};

END_NAMESPACE_GMCORE;

#endif
