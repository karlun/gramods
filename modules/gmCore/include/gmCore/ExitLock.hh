
#ifndef GRAMODS_CORE_EXITLOCK
#define GRAMODS_CORE_EXITLOCK

#include <gmCore/config.hh>

#include <memory>

BEGIN_NAMESPACE_GMCORE;

/**
   ExitLock allows different components to synchronize finalization of
   processing before the application is closed.

   Any component performing tasks that must finish before the
   application terminates should hold a shared_ptr to the singleton
   until they are done, and any component requesting the application
   to close should do so with via the singleton to make sure that the
   application does not close until all other components are done. It
   is thus important that this pointer is not held longer than
   absolutely necessary, since this will prevent the application from
   terminating when requested to.
*/
class ExitLock {

private:
  ExitLock();

public:
  ~ExitLock();

  /**
     Request an exit lock. Holding this lock will prevent the
     application from being terminated (by the ExitLock) until
     released. This will return a null pointer if some component
     already requested the ExitLock to terminate the application.
  */
  static std::shared_ptr<ExitLock> get();

  /**
     Makes the ExitLock terminate the application when all pointers to
     it goes out of scope. Observe that the application will continue
     to run at least until the shared pointer goes out of scope and
     possibly much longer.
  */
  void requestExit(int code);

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMCORE;

#endif
