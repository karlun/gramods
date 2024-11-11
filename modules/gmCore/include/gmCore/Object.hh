
#ifndef GRAMODS_CORE_OBJECT
#define GRAMODS_CORE_OBJECT

#include <gmCore/config.hh>

#include <memory>
#include <string>

BEGIN_NAMESPACE_GMCORE;

/**
   Base type for objects in the Gramods package for standardized
   handling of construction, initialization and destruction, and for
   setting up configuration based on configuration files, read using
   gmCore::Configuration.

   The most typical usage is
   \code
   gmCore::Configuration config(argc, argv);
   
   std::shared_ptr<MyClass> my_object;
   if (! config.getObject(my_object)) {
     GM_ERR("MyCode", "Cannot run without MyClass instance");
     exit(-1);
   }
   \endcode

   If gmCore::Configuration is not used, then the classes may be
   manually instantiated and initialized. The basic initialization
   procedure should then be as follows:

   1. Instantiate with empty constructor.
   2. Call setters to configure the object.
   3. Call initialize.
   4. Object is ready to be used.

   All three steps are performed by the Configuration tool and an
   Object extracted therefrom will be ready to be used.
*/
class Object
  : public std::enable_shared_from_this<Object> {

public:

  /**
     Initializes internal data.
  */
  Object() : is_initialized(false) {}

  /**
     Cleaning up internal data.
  */
  virtual ~Object() {}

  /**
     Called to initialize the Object. This should be called once only!

     Sub classes should override this method to make use of set
     parameter data. They should also always call its base class'
     implementation as well.
  */
  virtual void initialize() { is_initialized = true; }

  /**
     The visitor of a design pattern for automatic traversal.

     Example usage:

     ```
     MyVisitor visitor;
     node->accept(&visitor);
     ```
  */
  struct Visitor {
    /**
       This method is called for each visited objects.
    */
    virtual void apply(Object *node) { node->traverse(this); }
  };

  /**
     Calls the visitors apply method with this as argument.
  */
  virtual void accept(Visitor *visitor) { visitor->apply(this); }

  /**
     Calls the child Object's accept method, if such exist.
  */
  virtual void traverse(Visitor *visitor) {}

  /**
     Returns the default key for the Object when automatically
     instantiated in a Configuration, i.e. where it ends up in a
     parent Object. Default is "object", but this should be overriden
     by sub classes and can also be overridden in XML by using the
     attribute `KEY`, as in `KEY="view"`.

     The key can also be used to extract a specific object from a
     Configuration instance:
     \code
     gmCore::Configuration config(argc, argv);

     std::shared_ptr<MyClass> my_object;
     if (! config.getObjectByKey("view", my_object)) {
       GM_ERR("MyCode", "Cannot run without MyClass instance");
       exit(-1);
     }
     \endcode
  */
  virtual std::string getDefaultKey() { return "object"; }

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
