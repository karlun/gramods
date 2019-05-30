
#ifndef GRAMODS_CORE_OFACTORYNODE
#define GRAMODS_CORE_OFACTORYNODE

#include <gmCore/config.hh>
#include <gmCore/Object.hh>
#include <gmCore/Configuration.hh>

#include <map>
#include <string>
#include <typeinfo>

#include <assert.h>

BEGIN_NAMESPACE_GMCORE;

/**\def GM_OFI_DECLARE
   Macro for declaring the registration of OFactoryInformation as a
   class member. Put this in your class definition, under a public or
   protected access specifier to allow for inheritance.
*/
#define GM_OFI_DECLARE                                              \
  static gramods::gmCore::OFactory::OFactoryInformation _gm_ofi;//

/**\def GM_OFI_DEFINE(OFI, NAME)
   Macro for instantiating the registration of a OFactoryInformation
   declared with OFI_DECLARE.

   @param NAME The name of the class, which will also be the
   registered association string that refers to this class in the
   object factory.
*/
#define GM_OFI_DEFINE(NAME)                                             \
  gramods::gmCore::OFactory::OFactoryInformation                        \
  NAME::_gm_ofi(#NAME, new gramods::gmCore::OFactory::ObjectCreator<NAME>());

/**\def GM_OFI_DEFINE_ABSTRACT(OFI, NAME)
   Macro for instantiating the registration of a OFactoryInformation
   declared with OFI_DECLARE. A class registered with this macro
   cannot be instantiated, but may declare attributes.

   @param NAME The name of the class.
*/
#define GM_OFI_DEFINE_ABSTRACT(NAME)              \
  gramods::gmCore::OFactory::OFactoryInformation  \
  NAME::_gm_ofi(#NAME, nullptr);

/**\def GM_OFI_DEFINE_SUB(NAME, BASE)
   Macro for instantiating the registration of a OFactoryInformation
   declared with GM_OFI_DECLARE, with association with its base class'
   registration data.

   @param NAME The name of the class, which will also be the
   registered association string that refers to this class in the
   object factory.

   @param BASE The name of the base class.
*/
#define GM_OFI_DEFINE_SUB(NAME, BASE)                                   \
  gramods::gmCore::OFactory::OFactoryInformation                        \
  NAME::_gm_ofi(#NAME, new gramods::gmCore::OFactory::ObjectCreator<NAME>(), &BASE::_gm_ofi);

/**\def GM_OFI_DEFINE_ABSTRACT_SUB(NAME, BASE)
   Macro for instantiating the registration of a OFactoryInformation
   declared with GM_OFI_DECLARE, with association with its base class'
   registration data. A class registered with this macro cannot be
   instantiated, but may declare attributes.

   @param NAME The name of the class.
   @param BASE The name of the base class.
*/
#define GM_OFI_DEFINE_ABSTRACT_SUB(NAME, BASE)    \
  gramods::gmCore::OFactory::OFactoryInformation  \
  NAME::_gm_ofi(#NAME, nullptr, &BASE::_gm_ofi);

/**\def GM_OFI_PARAM(CLASS, NAME, TYPE, FUNC)
   Macro for registering a parameter setter to a OFactoryInformation
   node.

   namespace MyClassInternals {
     GM_OFI_DEFINE(MyClass);
     GM_OFI_PARAM(MyClass, file, std::string, MyClass::setFile);
   }

   @param CLASS The type of the class in which this setter resides.

   @param NAME The name to associate to the setter method, without
   quotes.

   @param TYPE The type of the variable set by this setter.

   @param FUNC The setter method.
*/
#define GM_OFI_PARAM(CLASS, NAME, TYPE, FUNC)                           \
  gramods::gmCore::OFactory::ParamSetterInsert gm_ofi_##CLASS##_param_##NAME \
  (&CLASS::_gm_ofi, #NAME,                                              \
   new gramods::gmCore::OFactory::ParamSetter<CLASS, TYPE>(&FUNC));

/**\def GM_OFI_POINTER(OFI, CLASS, NAME, TYPE, FUNC)
   Macro for registering a shared object setter to a
   OFactoryInformation node.

   GM_OFI_DEFINE(MyClass);
   GM_OFI_POINTER(MyClass, child, Node, MyClass::setChild);

   @param CLASS The type of the class in which this setter resides.

   @param NAME The name to associate to the setter method, without
   quotes.

   @param TYPE The class of the shared object. The type of the setter
   method is std::shared_ptr<TYPE>.

   @param FUNC The setter method, with signature
   void FUNC(std::shared_ptr<TYPE>)
*/
#define GM_OFI_POINTER(CLASS, NAME, TYPE, FUNC)                         \
  gramods::gmCore::OFactory::PointerSetterInsert gm_ofi_##CLASS##_pointer_##NAME \
  (&CLASS::_gm_ofi, #NAME,                                              \
   new gramods::gmCore::OFactory::PointerSetter<CLASS, TYPE>(&FUNC));

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

  struct ObjectCreatorBase {
    /** Creates and returns an instance of the template argument
        class. */
    virtual Object * create() = 0;
  };

public:

  struct OFactoryInformation {

    /**
       Registers the specified creator with the object factory and
       associates it with the provided name. The optional base
       parameter specifies the information of the base class for
       inheritance of setters.
    */
    OFactoryInformation(std::string name,
                        ObjectCreatorBase *creator,
                        OFactoryInformation *base = nullptr);

    /** Unregisters the class at the object factory. */
    ~OFactoryInformation();

    Object * create();

    void registerParamSetter(std::string name, ParamSetterBase *setter);

    void registerPointerSetter(std::string name, PointerSetterBase *setter);

    bool setParamValueFromString(Object *node, std::string name, std::string value);

    bool setPointerValue(Object *node, std::string name, std::shared_ptr<Object> ptr);

  private:

    const std::string name;
    ObjectCreatorBase * const creator;
    OFactoryInformation * const base;

    std::map<std::string, std::unique_ptr<ParamSetterBase>> param_setters;
    std::map<std::string, std::unique_ptr<PointerSetterBase>> pointer_setters;
  };

  struct ParamSetterInsert {
    ParamSetterInsert(OFactoryInformation *ofi,
                      std::string name,
                      ParamSetterBase *setter) {
      ofi->registerParamSetter(name, setter);
    }
  };

  struct PointerSetterInsert {
    PointerSetterInsert(OFactoryInformation *ofi,
                        std::string name,
                        PointerSetterBase *setter) {
      ofi->registerPointerSetter(name, setter);
    }
  };

  template<class Node, class T>
  struct ParamSetter : ParamSetterBase {

    ParamSetter(void (Node::*m)(T val))
      : method(m) {}

    void setValueFromString(Object *n, std::string s);

    void (Node::*method)(T val);
  };

  template<class Node>
  struct ParamSetter<Node, std::string> : ParamSetterBase {

    ParamSetter(void (Node::*m)(std::string val))
      : method(m) {}

    void setValueFromString(Object *n, std::string s);

    void (Node::*method)(std::string val);
  };

  template<class Node>
  struct ParamSetter<Node, bool> : ParamSetterBase {

    ParamSetter(void (Node::*m)(bool val))
      : method(m) {}

    void setValueFromString(Object *n, std::string s);

    void (Node::*method)(bool val);
  };

  template<class Node, class T>
  struct PointerSetter : PointerSetterBase {

    PointerSetter(void (Node::*m)(std::shared_ptr<T> ptr))
      : method(m) {}

    void setPointer(Object *n, std::shared_ptr<Object> ptr);

    void (Node::*method)(std::shared_ptr<T> ptr);
  };

  template<class Node>
  struct ObjectCreator : ObjectCreatorBase {
    /** Creates and returns an instance of the template argument
        class. */
    Object * create(){ return new Node; }
  };
  
  /** Creates and returns an instance of the class associated with the
      provided name, or NULL if no such name exists in the database. */
  static Object * createObject(std::string name);

private:

  static std::map<std::string,OFactoryInformation*>& getOFIByNameMap();

  static void registerOFI(std::string name, OFactoryInformation *info);
  static void unregisterOFI(std::string name);
  static OFactoryInformation* getOFI(std::string name);


  friend struct OFactoryInformation;

  friend class Configuration;
};


template<class Node, class T>
void OFactory::ParamSetter<Node, T>::setValueFromString
(Object *n, std::string s) {
  assert(dynamic_cast<Node*>(n) != nullptr);
  Node *node = static_cast<Node*>(n);
  std::stringstream ss(s);
  T val;
  ss >> val;

  if (!ss) {
    GM_ERR("Configuration", "cannot parse '" << s << "' as type " << typeid(T).name());
    throw new std::invalid_argument("type cannot parse string");
  }

  (node->*method)(val);
}

template<class Node>
void OFactory::ParamSetter<Node, std::string>::setValueFromString
(Object *n, std::string s) {
  assert(dynamic_cast<Node*>(n) != nullptr);
  Node *node = static_cast<Node*>(n);

  (node->*method)(s);
}

template<class Node>
void OFactory::ParamSetter<Node, bool>::setValueFromString
(Object *n, std::string s) {
  assert(dynamic_cast<Node*>(n) != nullptr);
  Node *node = static_cast<Node*>(n);

  if (s == "true" ||
      s == "True" ||
      s == "TRUE" ||
      s == "on" ||
      s == "On" ||
      s == "ON" ||
      s == "1") {
    (node->*method)(true);
  } else if (s == "false" ||
             s == "False" ||
             s == "FALSE" ||
             s == "off" ||
             s == "Off" ||
             s == "OFF" ||
             s == "0") {
    (node->*method)(false);
  } else {
    std::stringstream ss;
    ss << "Cannot convert " << s << " to boolean";
    throw std::invalid_argument(ss.str());
  }
}

template<class Node, class T>
void OFactory::PointerSetter<Node, T>::setPointer
(Object *n, std::shared_ptr<Object> ptr) {
  assert(dynamic_cast<Node*>(n) != nullptr);
  Node *node = static_cast<Node*>(n);
  std::shared_ptr<T> _ptr = std::dynamic_pointer_cast<T>(ptr);

  if (!_ptr) {
    GM_ERR("Configuration", "cannot cast " << typeid(ptr).name() << " to type " << typeid(T).name());
    throw new std::invalid_argument("cannot cast pointer");
  }

  (node->*method)(_ptr);
}

END_NAMESPACE_GMCORE;

#endif
