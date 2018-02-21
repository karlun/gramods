
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

/**\def OFI_PARAM(OFI, CLASS, NAME, TYPE, FUNC)
   Macro for registering a parameter setter to a OFactoryInformation
   node.

   namespace MyClassInternals {
     OFI_CREATE(OFI, MyClass);
     OFI_SETTER(OFI, MyClass, file, std::string, MyClass::setFile);
   }

   @param OFI The statically instantiated OFactoryInformation of the
   Object for which to register setters.

   @param CLASS The type of the class in which this setter resides.

   @param NAME The name to associate to the setter method, without
   quotes.

   @param TYPE The type of the variable set by this setter.

   @param FUNC The setter method.
*/
#define OFI_PARAM(OFI, CLASS, NAME, TYPE, FUNC)                         \
  gramods::gmConfig::OFactory::ParamSetterInsert OFI##NAME              \
  (&OFI, #NAME, new gramods::gmConfig::OFactory::OFactoryInformation<CLASS>::ParamSetter<TYPE>(&FUNC));

/**\def OFI_POINTER(OFI, CLASS, NAME, TYPE, FUNC)
   Macro for registering a shared object setter to a
   OFactoryInformation node.

   namespace MyClassInternals {
     OFI_CREATE(OFI, MyClass);
     OFI_SETTER(OFI, MyClass, file, std::string, MyClass::setFile);
   }

   @param OFI The statically instantiated OFactoryInformation of the
   Object for which to register setters.

   @param CLASS The type of the class in which this setter resides.

   @param NAME The name to associate to the setter method, without
   quotes.

   @param TYPE The class of the shared object. The type of the setter
   method is std::shared_ptr<TYPE>.

   @param FUNC The setter method, with signature
   void FUNC(std::shared_ptr<TYPE>)
*/
#define OFI_POINTER(OFI, CLASS, NAME, TYPE, FUNC)                       \
  gramods::gmConfig::OFactory::PointerSetterInsert OFI##NAME            \
  (&OFI, #NAME, new gramods::gmConfig::OFactory::OFactoryInformation<CLASS>::PointerSetter<TYPE>(&FUNC));

/**
   This is an object factory for classes with Object as base type,
   instantiating them by name and calling their registered setters
   based on configuration XML DOM data.
*/
class OFactory {

  struct ParamSetterBase {
    virtual void setValueFromString(Object *n, std::string s) = 0;
  };

  struct PointerSetterBase {
    virtual void setPointer(Object *n, std::shared_ptr<Object> o) = 0;
  };

  struct OFactoryInformationBase {

    virtual Object * create() = 0;

    void registerParamSetter(std::string name, ParamSetterBase *setter) {
      assert(param_setters.count(name) == 0);
      param_setters[name].reset(setter);
    }

    void registerPointerSetter(std::string name, PointerSetterBase *setter) {
      assert(pointer_setters.count(name) == 0);
      pointer_setters[name].reset(setter);
    }

    virtual bool setParamValueFromString(Object *n, std::string name, std::string value) = 0;

    virtual bool setPointerValue(Object *n, std::string name, std::shared_ptr<Object> ptr) = 0;

    std::map<std::string, std::unique_ptr<ParamSetterBase>> param_setters;
    std::map<std::string, std::unique_ptr<PointerSetterBase>> pointer_setters;
  };

public:

  struct ParamSetterInsert {
    ParamSetterInsert(OFactoryInformationBase *ofi,
                      std::string name,
                      ParamSetterBase *setter) {
      ofi->registerParamSetter(name, setter);
    }
  };

  struct PointerSetterInsert {
    PointerSetterInsert(OFactoryInformationBase *ofi,
                        std::string name,
                        PointerSetterBase *setter) {
      ofi->registerPointerSetter(name, setter);
    }
  };

  /** Creating a static instance of this class with the concrete class
      as template argument will register that class with this object
      factory. */
  template<class Node>
  struct OFactoryInformation
    : OFactoryInformationBase {

    template<class T>
    struct ParamSetter : ParamSetterBase {

      ParamSetter(void (Node::*m)(T val))
        : method(m) {}

      void setValueFromString(Object *n, std::string s);

      void (Node::*method)(T val);
    };

    template<class T>
    struct PointerSetter : PointerSetterBase {

      PointerSetter(void (Node::*m)(std::shared_ptr<T> ptr))
        : method(m) {}

      void setPointer(Object *n, std::shared_ptr<Object> ptr);

      void (Node::*method)(std::shared_ptr<T> ptr);
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

    bool setParamValueFromString(Object *node, std::string name, std::string value) {
      if (param_setters.count(name) == 0) return false;
      param_setters[name]->setValueFromString(node, value);
      return true;
    }

    bool setPointerValue(Object *node, std::string name, std::shared_ptr<Object> ptr) {
      if (pointer_setters.count(name) == 0) return false;
      pointer_setters[name]->setPointer(node, ptr);
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
void OFactory::OFactoryInformation<Node>::ParamSetter<T>::setValueFromString(Object *n, std::string s) {
  assert(dynamic_cast<Node*>(n) != nullptr);
  Node *node = static_cast<Node*>(n);
  std::stringstream ss(s);
  T val;
  ss >> val;

  if (!ss)
    GRAMODS_THROW(std::invalid_argument, "cannot cast '" << s << "' to type " << typeid(T).name());

  (node->*method)(val);
}

template<class Node>
template<class T>
void OFactory::OFactoryInformation<Node>::PointerSetter<T>::setPointer(Object *n, std::shared_ptr<Object> ptr) {
  assert(dynamic_cast<Node*>(n) != nullptr);
  Node *node = static_cast<Node*>(n);
  std::shared_ptr<T> _ptr = std::dynamic_pointer_cast<T>(ptr);

  if (!_ptr)
    GRAMODS_THROW(std::invalid_argument, "cannot cast " << typeid(ptr).name() << " to type " << typeid(T).name());

  (node->*method)(_ptr);
}

END_NAMESPACE_GMCONFIG

#endif
