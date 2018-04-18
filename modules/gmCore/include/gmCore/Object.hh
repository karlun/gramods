
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
*/
struct Object
  : public std::enable_shared_from_this<Object> {

  /**
     Cleaning up internal data.
  */
  virtual ~Object() {}

  /**
     Called to initialize the Object. Sub classes to override this to
     make use of set parameter data. This should be called once only!
  */
  virtual void initialize() {}
};

END_NAMESPACE_GMCORE;

#endif