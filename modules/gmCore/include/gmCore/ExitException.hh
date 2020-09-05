
#ifndef GRAMODS_CORE_EXITEXCEPTION
#define GRAMODS_CORE_EXITEXCEPTION

#include <gmCore/config.hh>

#include <exception>
#include <string>

BEGIN_NAMESPACE_GMCORE;

/**
   Standard exception for exiting the application with a specified
   exit code. Top level execution of gramods objects should catch
   this exception and exit the application with the given exit code.

   Exiting the application is an exception because normally the
   application should be running. It is not always practical to handle
   exiting as a normal, non-exceptional behavior.
*/
struct ExitException {

  /**
     Creates an exit exception with the specified exit code, or an
     exit code of 0 (zero) if not specified.
  */
  ExitException(int exit_code = 0)
    : exit_code(exit_code) {}

  /**
     The exit code of the exit exception.
  */
  const int exit_code;
};

END_NAMESPACE_GMCORE;

#endif
