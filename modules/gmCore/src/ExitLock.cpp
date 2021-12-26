
#include <gmCore/ExitLock.hh>

#include <gmCore/Console.hh>

#include <stdlib.h>

BEGIN_NAMESPACE_GMCORE;

struct ExitLock::Impl {
  bool do_exit = false;
  int return_code = 0;
};

ExitLock::ExitLock()
  : _impl(std::make_unique<Impl>()) {}

ExitLock::~ExitLock() {
  GM_DBG2("ExitLock", "Lock released by all.");
  if(_impl->do_exit) ::exit(_impl->return_code);
}

std::shared_ptr<ExitLock> ExitLock::get() {
  static std::mutex lock;
  std::lock_guard<std::mutex> guard(lock);

  static std::weak_ptr<ExitLock> singleton;

  std::shared_ptr<ExitLock> locked = singleton.lock();
  if (!locked) {
    GM_DBG2("ExitLock", "Locking");
    locked.reset(new ExitLock());
    singleton = locked;
    return locked;
  }

  if (locked->_impl->do_exit) {
    GM_DBG2("ExitLock", "Refusing lock since exit has been requested.");
    return nullptr;
  }

  GM_DBG2("ExitLock", "Sharing lock.");
  return locked;
}

void ExitLock::requestExit(int code) {
  GM_DBG2("ExitLock", "Exit requested with code " << code << ".");
  _impl->do_exit = true;
  if (code) {
    if (_impl->return_code && _impl->return_code != code)
      GM_WRN("ExitLock",
             "Component requested exit with return code "
                 << code << " while return code was set to "
                 << _impl->return_code << "!");
    else
      _impl->return_code = code;
  }
}

END_NAMESPACE_GMCORE;
