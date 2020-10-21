
#ifndef GRAMODS_CORE_PRECONDITIONVIOLATION
#define GRAMODS_CORE_PRECONDITIONVIOLATION

#include <gmCore/RuntimeException.hh>

#include <string>

BEGIN_NAMESPACE_GMCORE;

/**
   Standard exception for violation of pre conditions in a call to a
   function or object. In well written software this exception is
   never thrown.
*/
struct PreConditionViolation : RuntimeException {

  /**
     Creates an exception with a clarification message.
  */
  PreConditionViolation(std::string what)
    : RuntimeException(what) {}
};

END_NAMESPACE_GMCORE;

#endif
