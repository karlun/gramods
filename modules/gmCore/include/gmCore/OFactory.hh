
#ifndef GRAMODS_CORE_OFACTORYNODE
#define GRAMODS_CORE_OFACTORYNODE

#include <gmCore/config.hh>
#include <gmCore/Object.hh>
#include <gmCore/Stringify.hh>
#include <gmCore/InvalidArgument.hh>

#include <map>
#include <string>
#include <filesystem>
#include <iomanip>
#include <typeinfo>
#include <iostream>
#include <sstream>

#include <assert.h>

BEGIN_NAMESPACE_GMCORE;

class Configuration;

/**
   \def GM_OFI_DECLARE
   Macro for declaring the registration of OFactoryInformation as a
   class member. Put this in your class definition, under a public or
   protected access specifier to allow for inheritance.
*/
#define GM_OFI_DECLARE                                              \
  static gramods::gmCore::OFactory::OFactoryInformation _gm_ofi;//

/**
   \def GM_OFI_DEFINE(NAME)
   Macro for instantiating the registration of a OFactoryInformation
   declared with OFI_DECLARE.

   @param NAME The name of the class, which will also be the
   registered association string that refers to this class in the
   object factory.
*/
#define GM_OFI_DEFINE(NAME)                                             \
  gramods::gmCore::OFactory::OFactoryInformation                        \
  NAME::_gm_ofi(#NAME, new gramods::gmCore::OFactory::ObjectCreator<NAME>());

/**
   \def GM_OFI_DEFINE_ABSTRACT(NAME)
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

/**
   \def GM_OFI_DEFINE_ABSTRACT_SUB(NAME, BASE)
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

/**
   \def GM_OFI_PARAM(CLASS, NAME, TYPE, FUNC)
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
#define GM_OFI_PARAM(CLASS, NAME, TYPE, FUNC)                                  \
  gramods::gmCore::OFactory::ParamSetterInsert gm_ofi_##CLASS##_param_##NAME(  \
      &CLASS::_gm_ofi,                                                         \
      #NAME,                                                                   \
      new gramods::gmCore::OFactory::ParamSetter<CLASS, TYPE>(&FUNC));

/**
   \def GM_OFI_PARAM2(CLASS, NAME, TYPE, FUNC)
   Macro for registering a parameter setter to a OFactoryInformation
   node.

   namespace MyClassInternals {
     GM_OFI_DEFINE(MyClass);
     GM_OFI_PARAM2(MyClass, file, std::string, setFile);
   }

   @param CLASS The type of the class in which this setter resides.

   @param NAME The name to associate to the setter method, without
   quotes.

   @param TYPE The type of the variable set by this setter.

   @param FUNC The setter method in the class CLASS.
*/
#define GM_OFI_PARAM2(CLASS, NAME, TYPE, FUNC)                                 \
  gramods::gmCore::OFactory::ParamSetterInsert gm_ofi_##CLASS##_param_##NAME(  \
      &CLASS::_gm_ofi,                                                         \
      #NAME,                                                                   \
      new gramods::gmCore::OFactory::ParamSetter<CLASS, TYPE>(&CLASS::FUNC));

/**
   \def GM_OFI_POINTER(CLASS, NAME, TYPE, FUNC)
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
   \def GM_OFI_POINTER2(CLASS, NAME, TYPE, FUNC)
   Macro for registering a shared object setter to a
   OFactoryInformation node.

   GM_OFI_DEFINE(MyClass);
   GM_OFI_POINTER2(MyClass, child, Node, setChild);

   @param CLASS The type of the class in which this setter resides.

   @param NAME The name to associate to the setter method, without
   quotes.

   @param TYPE The class of the shared object. The type of the setter
   method is std::shared_ptr<TYPE>.

   @param FUNC The setter method in the class CLASS, with signature
   void FUNC(std::shared_ptr<TYPE>)
*/
#define GM_OFI_POINTER2(CLASS, NAME, TYPE, FUNC)                         \
  gramods::gmCore::OFactory::PointerSetterInsert gm_ofi_##CLASS##_pointer_##NAME \
  (&CLASS::_gm_ofi, #NAME,                                              \
   new gramods::gmCore::OFactory::PointerSetter<CLASS, TYPE>(&CLASS::FUNC));

/**
   This is an object factory for classes with Object as base type,
   instantiating them by name and calling their registered setters
   based on configuration XML DOM data.

   The following macros are useful for correctly setting up a class
   and its members for use with OFactory.

   - #GM_OFI_DECLARE
   - #GM_OFI_DEFINE(NAME)
   - #GM_OFI_DEFINE_ABSTRACT(NAME)
   - #GM_OFI_DEFINE_SUB(NAME, BASE)
   - #GM_OFI_DEFINE_ABSTRACT_SUB(NAME, BASE)
   - #GM_OFI_PARAM(CLASS, NAME, TYPE, FUNC)
   - #GM_OFI_PARAM2(CLASS, NAME, TYPE, FUNC)
   - #GM_OFI_POINTER(CLASS, NAME, TYPE, FUNC)
   - #GM_OFI_POINTER2(CLASS, NAME, TYPE, FUNC)
*/
class OFactory {

  /**
     Base for parameter setters, that set value to an attribute of a
     specified object from a string. The setter will call a member
     method to set the parsed value.
   */
  struct ParamSetterBase {
    virtual void setValueFromString(Object *n, std::string s) const = 0;
  };

  /**
     Base for pointer setters, that assigns a pointer to another
     object as an attribute of an object. The setter will call a
     member method to set the parsed value.
   */
  struct PointerSetterBase {
    virtual void setPointer(Object *n, std::shared_ptr<Object> o) const = 0;
  };

  /**
     Base for object creators, that instantiate an object type.
  */
  struct ObjectCreatorBase {
    /** Creates and returns an instance of the template argument
        class. */
    virtual Object * create() const = 0;
  };

public:

  /**
     Registrator of information required by the object factory. By
     instantiating this class a creator object is associated with a
     name. The information object can also be populated with attribute
     setters for setting attribute pointers and values from strings.
  */
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

    /**
       Unregisters the class at the object factory.
    */
    ~OFactoryInformation();

    /**
       Instantiates the class associated with this OFI and returns a
       raw pointer to this object.
    */
    Object * create() const;

    /**
       Registers a parameter setter with a specific attribute name.
    */
    void registerParamSetter(std::string name, ParamSetterBase *setter);

    /**
       Registers a pointer setter with a specific attribute name.
    */
    void registerPointerSetter(std::string name, PointerSetterBase *setter);

    /**
       Finds a parameter setter for the specified attribute name and
       calls it to set that attribute value for the specified object
       to the value encoded by the specified string value. Returns
       false if there is no setter associated with the specified
       attribute name.
    */
    bool setParamValueFromString(Object *node, std::string name, std::string value) const;

    /**
       Finds a pointer setter for the specified attribute name and
       calls it to set that attribute pointer for the specified object
       to the specified pointer value. Returns false if there is no
       setter associated with the specified attribute name.
    */
    bool setPointerValue(Object *node, std::string name, std::shared_ptr<Object> ptr) const;

  private:

    const std::string name;
    const std::unique_ptr<const ObjectCreatorBase> creator;
    const OFactoryInformation *const base;

    std::map<std::string, std::unique_ptr<ParamSetterBase>> param_setters;
    std::map<std::string, std::unique_ptr<PointerSetterBase>> pointer_setters;
  };

  /**
     Convenience class that registers a parameter setter upon
     instantiation.
  */
  struct ParamSetterInsert {

    /**
       Constructor registering the specified parameter setter with the
       specified OFI, associating it with the specified name.
    */
    ParamSetterInsert(OFactoryInformation *ofi,
                      std::string name,
                      ParamSetterBase *setter) {
      ofi->registerParamSetter(name, setter);
    }
  };

  /**
     Convenience class that registers a pointer setter upon
     instantiation.
  */
  struct PointerSetterInsert {

    /**
       Constructor registering the specified pointer setter with the
       specified OFI, associating it with the specified name.
    */
    PointerSetterInsert(OFactoryInformation *ofi,
                        std::string name,
                        PointerSetterBase *setter) {
      ofi->registerPointerSetter(name, setter);
    }
  };

  /**
     General parameter setter, templated to determine the type to set
     value for. Any type, even types unknown to the library, can be
     used, as long as it supports the istream operator. Include the
     header code defining the istream operator before including this
     header, to make sure that the template class is instantiated
     against that type.
  */
  template<class Node, class T>
  struct ParamSetter : ParamSetterBase {

    ParamSetter(void (Node::*m)(T val))
      : method(m) {}

    void setValueFromString(Object *n, std::string s) const;

    void (Node::*method)(T val);
  };

  /**
     Specialization parameter setter for string type, to avoid
     tokenization of strings with white space.
  */
  template<class Node>
  struct ParamSetter<Node, std::string> : ParamSetterBase {

    ParamSetter(void (Node::*m)(std::string val))
      : method(m) {}

    void setValueFromString(Object *n, std::string s) const;

    void (Node::*method)(std::string val);
  };

  /**
     Specialization parameter setter for std::filesystem::path type,
     to avoid tokenization of paths with white space.
  */
  template<class Node>
  struct ParamSetter<Node, std::filesystem::path> : ParamSetterBase {

    ParamSetter(void (Node::*m)(std::filesystem::path val))
      : method(m) {}

    void setValueFromString(Object *n, std::string s) const;

    void (Node::*method)(std::filesystem::path val);
  };

  /**
     Specialization parameter setter for bool type, to allow named
     states, such as "on" and "true".
  */
  template<class Node>
  struct ParamSetter<Node, bool> : ParamSetterBase {

    ParamSetter(void (Node::*m)(bool val))
      : method(m) {}

    void setValueFromString(Object *n, std::string s) const;

    void (Node::*method)(bool val);
  };

  /**
     General pointer setter, templated to determine the type of the
     pointer.
  */
  template<class Node, class T>
  struct PointerSetter : PointerSetterBase {

    PointerSetter(void (Node::*m)(std::shared_ptr<T> ptr))
      : method(m) {}

    void setPointer(Object *n, std::shared_ptr<Object> ptr) const;

    void (Node::*method)(std::shared_ptr<T> ptr);
  };

  /**
     Actual object creator, templated for the type to instantiate.
  */
  template<class Node>
  struct ObjectCreator : ObjectCreatorBase {
    /** Creates and returns an instance of the template argument
        class. */
    Object *create() const { return new Node; }
  };
  
  /** Creates and returns an instance of the class associated with the
      provided name, or NULL if no such name exists in the database. */
  static Object * createObject(std::string name);

private:

  static std::map<std::string, OFactoryInformation *> &getOFIByNameMap();

  static void registerOFI(std::string name, OFactoryInformation *info);
  static void unregisterOFI(std::string name);
  static OFactoryInformation* getOFI(std::string name);


  friend struct OFactoryInformation;

  friend class Configuration;
};


template<class Node, class T>
void OFactory::ParamSetter<Node, T>::setValueFromString
(Object *n, std::string s) const {
  assert(dynamic_cast<Node*>(n) != nullptr);
  Node *node = static_cast<Node*>(n);
  std::stringstream ss(s);
  T val;
  ss >> std::setbase(0) >> val;

  if (!ss)
    throw gmCore::InvalidArgument(GM_STR("cannot parse '" << s << "' as type " << typeid(T).name()));

  (node->*method)(val);
}

template<class Node>
void OFactory::ParamSetter<Node, std::string>::setValueFromString
(Object *n, std::string s) const {
  assert(dynamic_cast<Node*>(n) != nullptr);
  Node *node = static_cast<Node*>(n);

  (node->*method)(s);
}

template<class Node>
void OFactory::ParamSetter<Node, std::filesystem::path>::setValueFromString
(Object *n, std::string s) const {
  assert(dynamic_cast<Node*>(n) != nullptr);
  Node *node = static_cast<Node*>(n);

  (node->*method)(s);
}

template<class Node>
void OFactory::ParamSetter<Node, bool>::setValueFromString
(Object *n, std::string s) const {
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
    throw gmCore::InvalidArgument(ss.str());
  }
}

template<class Node, class T>
void OFactory::PointerSetter<Node, T>::setPointer
(Object *n, std::shared_ptr<Object> ptr) const {
  assert(dynamic_cast<Node*>(n) != nullptr);
  Node *node = static_cast<Node*>(n);
  std::shared_ptr<T> _ptr = std::dynamic_pointer_cast<T>(ptr);

  if (!_ptr)
    throw gmCore::InvalidArgument(GM_STR("cannot cast " << typeid(ptr).name() << " to type " << typeid(T).name()));

  (node->*method)(_ptr);
}

END_NAMESPACE_GMCORE;

#endif
