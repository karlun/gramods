
#ifndef GRAMODS_CORE_CONSOLELEVEL
#define GRAMODS_CORE_CONSOLELEVEL

BEGIN_NAMESPACE_GMCORE;

/**
   The level of importance, used by Console and MessageSink.
*/
enum struct ConsoleLevel {

  /**
     Error message level. Messages with this level should be very
     sparse, since most error information needs to be provided to the
     users via the user interface instead.
  */
  error = 0,

  /**
     Warning message. Messages with this level should be sparse, and
     indicate something that might be normal, but might result in
     unexpected behavior further on.
  */
  warning = 1,

  /**
     Information message. Messages with this level report on high
     level internal processes. Do not send more than a few at a time
     and fairly seldom.
  */
  information = 2,

  /**
     Detailed information message. Messages with this level may arrive
     at a higher rate, inside loops, but should not slow down the
     application.
  */
  verbose_information = 3,

  /**
     Very detailed information message. Messages with this level may
     arrive at a high rate, inside loops, and may be so many that they
     can be expected to slow down the application.
  */
  very_verbose_information = 4
};

END_NAMESPACE_GMCORE;

#endif
