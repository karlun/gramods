
#ifndef GRAMODS_CONFIG_OFACTORYNODE
#define GRAMODS_CONFIG_OFACTORYNODE

#include <map>
#include <string>

#include <gmConfig/config.hh>
#include <gmConfig/Configuration.hh>
#include <gmConfig/Debug.hh>

BEGIN_NAMESPACE_GMCONFIG

/**
   This is an object factory and the base type for all classes that
   can be instanciated by name and configured through XML DOM data.

   A subclass will implement the pure virtual function (configure) to
   provide a means to configure its data, and create a static instance
   of OFactoryInformation with itself as template argument, and that
   will register the class with the factory.  */
class OFactoryNode {

  struct OFactoryInformationBase {
    virtual OFactoryNode * create() = 0;
  };

public:

  /** Creating a static instance of this class with the concrete class
      as template argument will register that class with this object
      factory. */
  template<class Node>
  struct OFactoryInformation
    : OFactoryInformationBase {

    /** Registers the template argument class with the object factory
        and associates it with the provided name. */
    OFactoryInformation(std::string name) : name(name) {
      OFactoryNode::registerOFI(name, this);
      std::cerr << name << " registered" << std::endl;
    }

    /** Unregisters the template argument class. */
    ~OFactoryInformation(){
      OFactoryNode::unregisterOFI(name);
      std::cerr << name << " unregistered" << std::endl;
    }

    /** Creates and returns an instance of the template argument
        class. */
    OFactoryNode * create(){ return new Node; }

    const std::string name;

  private:
    OFactoryInformation();
  };

  /** Creates and returns an instance of the class associated with the
      provided name, or NULL if no such name exists in the database. */
  static OFactoryNode * createObject(std::string name);

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

};

END_NAMESPACE_GMCONFIG

#endif
