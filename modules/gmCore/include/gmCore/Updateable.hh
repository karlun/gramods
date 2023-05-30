
#ifndef GRAMODS_CORE_UPDATEABLE
#define GRAMODS_CORE_UPDATEABLE

#include <gmCore/config.hh>

#include <chrono>
#include <memory>
#include <optional>

BEGIN_NAMESPACE_GMCORE;

/**
   The Updateable class defines an interface for objects that may be
   updated, for example each execution frame. This may be an animator
   or a network connection.
*/
class Updateable {

public:

  typedef std::chrono::steady_clock clock;

  /**
   * Configures the Updateable properties to the specified
   * priority. Upon call to updateAll all instances of Updateable will
   * be called in turn based on their priority. Highest priority will
   * be called first and lowest last. Instances with the same priority
   * will be called in the order of instantiation.
   */
  Updateable(int priority = 0);

  virtual ~Updateable();

  /**
     Updates all currently instanciated updateable objects.
  */
  static void updateAll(clock::time_point t = clock::now(),
                        std::optional<size_t> frame = std::nullopt);

  /**
     Called by updateAll to make the object up-to-date.
  */
  virtual void update(clock::time_point t, size_t frame) = 0;

private:

  struct Impl;
  Impl * _impl;

};


END_NAMESPACE_GMCORE;

#endif
