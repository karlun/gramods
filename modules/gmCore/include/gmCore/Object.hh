
#ifndef GRAMODS_CORE_OBJECT
#define GRAMODS_CORE_OBJECT

#include <gmCore/config.hh>

#include <memory>

BEGIN_NAMESPACE_GMCORE;

/**
   Base type for objects in the Gramods package, for easier handling
   of construction, initialization and destruction. The basic
   initialization procedure should be as follows:

   1. Instantiate with empty constructor.
   2. Call setters to configure the object.
   3. Call initialize.
   4. Object is ready to be used.

   All three steps are performed by the Configuration tool and an
   Object extracted therefrom will be ready to be used.
*/
struct Object
  : public std::enable_shared_from_this<Object> {

  /**
     Initializes internal data.
  */
  Object() : is_initialized(false) {}

  /**
     Cleaning up internal data.
  */
  virtual ~Object() {}

  /**
     Called to initialize the Object. Sub classes to override this to
     make use of set parameter data. This should be called once only!
  */
  virtual void initialize() { is_initialized = true; }

  /**
     Returns true if the Object is initialized. Sub classes must call
     Object::initialize() if and only if the initialization succeeded.
  */
  bool isInitialized() { return is_initialized; }

private:

  bool is_initialized;
};

END_NAMESPACE_GMCORE;

#endif
