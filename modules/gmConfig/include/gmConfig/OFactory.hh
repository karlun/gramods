
#ifndef GRAMODS_CONFIG_OFACTORYNODE
#define GRAMODS_CONFIG_OFACTORYNODE

#include <gmConfig/config.hh>
#include <gmConfig/Object.hh>
#include <gmConfig/Configuration.hh>
#include <gmConfig/Debug.hh>

#include <map>
#include <string>
#include <typeinfo>

#include <assert.h>

BEGIN_NAMESPACE_GMCONFIG

/** \def OFI_SETTER(OFI, NAME, TYPE, FUNC)
   Macro for registering a setter to a OFactoryInformation node.

   BEGIN_NAMESPACE_GMCONFIG
   namespace ImportLibraryInternals {
     OFactory::OFactoryInformation<ImportLibrary> OFI("ImportLibrary");
     OFI_SETTER(OFI, file, std::string, ImportLibrary::setFile);
   }
   END_NAMESPACE_GMCONFIG

   @param OFI The statically instantiated OFactoryInformation of the
   Object for which to register setters.

   @param NAME The name to associate to the setter method, without
   quotes.

   @param TYPE The type of the variable set by this setter.

   @param FUNC The setter method.
*/
#define OFI_SETTER(OFI, NAME, TYPE, FUNC)       \
  OFactory::SetterInsert OFI##NAME (&OFI, #NAME, new OFactory::OFactoryInformation<ImportLibrary>::Setter<TYPE>(&FUNC));

/**
   This is an object factory for classes with Object as base type,
   instantiating them by name and calling their registered setters
   based on configuration XML DOM data.
*/
class OFactory {

  struct SetterBase {
    virtual void setValueFromString(Object *n, std::string s) = 0;
  };

  struct OFactoryInformationBase {

    virtual Object * create() = 0;

    void registerSetter(std::string name, SetterBase *setter) {
      assert(setters.count(name) == 0);
      setters[name].reset(setter);
    }

    virtual bool setFieldValueFromString(Object *n, std::string name, std::string value) = 0;

    std::map<std::string, std::unique_ptr<SetterBase>> setters;
  };

public:

  struct SetterInsert {
    SetterInsert(OFactoryInformationBase *ofi,
                 std::string name,
                 SetterBase *setter) {
      ofi->registerSetter(name, setter);
    }
  };

  /** Creating a static instance of this class with the concrete class
      as template argument will register that class with this object
      factory. */
  template<class Node>
  struct OFactoryInformation
    : OFactoryInformationBase {

    template<class T>
    struct Setter : SetterBase {

      Setter(void (Node::*m)(T val))
        : method(m) {}

      void setValueFromString(Object *n, std::string s);

      void (Node::*method)(T val);
    };

    /** Registers the template argument class with the object factory
        and associates it with the provided name. */
    OFactoryInformation(std::string name) : name(name) {
      OFactory::registerOFI(name, this);
    }

    /** Unregisters the template argument class. */
    ~OFactoryInformation(){
      OFactory::unregisterOFI(name);
    }

    /** Creates and returns an instance of the template argument
        class. */
    Object * create(){ return new Node; }

    bool setFieldValueFromString(Object *node, std::string name, std::string value) {
      if (setters.count(name) == 0) return false;
      setters[name]->setValueFromString(node, value);
      return true;
    }

    const std::string name;

  private:
    OFactoryInformation();
  };

  /** Creates and returns an instance of the class associated with the
      provided name, or NULL if no such name exists in the database. */
  static Object * createObject(std::string name);

  /** Configure the object using the provided XML DOM tree. This
      function must be implemented by a concrete class. */
  virtual void configure(const Configuration &config) = 0;

private:

  static std::map<std::string,OFactoryInformationBase*>& getOFIByNameMap();

  static void registerOFI(std::string name, OFactoryInformationBase *info);
  static void unregisterOFI(std::string name);
  static OFactoryInformationBase* getOFI(std::string name);


  template<class Node>
  friend struct OFactoryInformation;

  friend class Configuration;
};


template<class Node>
template<class T>
void OFactory::OFactoryInformation<Node>::Setter<T>::setValueFromString(Object *n, std::string s) {
  assert(dynamic_cast<Node*>(n) != nullptr);
  Node *node = static_cast<Node*>(n);
  std::stringstream ss(s);
  T val;
  ss >> val;

  if (!ss)
    GRAMODS_THROW(std::invalid_argument, "cannot cast '" << s << "' to type " << typeid(T).name());

  (node->*method)(val);
}

END_NAMESPACE_GMCONFIG

#endif
