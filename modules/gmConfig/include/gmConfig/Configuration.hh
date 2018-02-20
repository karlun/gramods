
#ifndef GRAMODS_CONFIG_CONFIGURATION
#define GRAMODS_CONFIG_CONFIGURATION

#include <gmConfig/config.hh>
#include <gmConfig/Debug.hh>

#include <vector>
#include <string>
#include <algorithm>
#include <memory>

namespace tinyxml2 {
  struct XMLNode;
  struct XMLElement;
}

BEGIN_NAMESPACE_GMCONFIG

class Object;

/**
   A wrapper for the XML parser that also creates and configures the
   system objects and holds temporary references to them for easy
   retrieval. This unit will build an internal tree structure of
   objects that can be easily retrieved by name or by type, and lists
   of parameters for each object.

   Typical usage:

   int width = DEFAULT_WIDTH;
   config->getParam("width", width);

 */
class Configuration {

public:

  /**
      Creates an empty configuration.
   */
  Configuration();

  /**
     Loads an XML string, create objects as specified by the XML data
     and configure the objects.
   */
  Configuration(std::string config);

  /**
     Read the XML data, create objects as specified by the XML data
     and configure the objects.
   */
  Configuration(tinyxml2::XMLNode *node);

  /**
     Cleans up and checks that all configuration variables have been read.
   */
  ~Configuration();

  /**
     Check if a specific parameter is available.
   */
  bool hasParam(const std::string &name);

  /**
     Retrieve a specified parameter of a specified type. This function
     will use stringstream to parse most types but will need
     specializations to parse non-standard types. If the parameter is
     not found, the value argument will not be changed.

     @param[in] name name of the parameter to retrieve
     @param[out] value the parameter value, if set
     @return false if the parameter is not set or if the value cannot
     be casted to the specified type.
   */
  template<class T>
  bool getParam(const std::string &name, T &value) const;

  /**
     Set a parameter value.
   */
  template<class T>
  void setParam(const std::string &name, const T &value);

  /**
     Retrieve a specified parameter as string.

     @param[in] name name of the parameter to retrieve
     @param[out] value the parameter value, if set
     @return false if the parameter is not set
   */
  bool getParamAsString(const std::string &name, std::string &value) const;

  /**
     Retrieve an object by its name.

     @param[in] name name of the parameter to retrieve
     @param[out] ptr a pointer to the object, if set
     @return false if no object was found by that name or if it could
     not be casted to the specified type.
   */
  template<class T>
  bool getObject(const std::string &name, std::shared_ptr<T> &ptr) const;

  /**
     Retrieve an object by its type. If there are multiple objects of
     the same type, there is no saying which will be retrieved with
     this function.

     @param[out] ptr a pointer to the object, if set
     @return false if no object was found by that name or if it could
     not be casted to the specified type.
   */
  template<class T>
  bool getObject(std::shared_ptr<T> &ptr) const;

  /**
     Set an object.
   */
  template<class T>
  void setObject(const std::string &name, std::shared_ptr<T> ptr);

  /**
     Set an object.
   */
  template<class T>
  void setObject(std::shared_ptr<T> ptr){ setObject(typeid(T).name(), ptr); }

  /**
     Retrieve all objects of a specified type. This function will
     append the retrieved objects to the specified list. Use
     Object as type to retrieve all objects.

     @param[out] value a vector of pointer to fill with objects
     @return the number of objects added to the list.
   */
  template<class T>
  int getAllObjects(std::vector<std::shared_ptr<T>> &value) const;

private:

  void load(tinyxml2::XMLNode *node);

  struct parameter_t {
    parameter_t(const std::string &value = "")
      : checked(false), value(value) {}
    std::string value;
    bool checked;
  };

  typedef std::map<std::string, parameter_t> parameter_list;

  std::multimap<std::string, std::shared_ptr<Object>> child_objects;
  parameter_list parameters;

  void parse_param(tinyxml2::XMLElement *element);

};


template<class T>
int Configuration::getAllObjects(std::vector<std::shared_ptr<T>> &value) const {

  Configuration *_this = const_cast<Configuration*>(this);

  int original_size = value.size();

  for( std::map<std::string, std::shared_ptr<Object>>::iterator it
         = _this->child_objects.begin() ;
       it != _this->child_objects.end() ; ++it ){

    std::shared_ptr<T> node = std::dynamic_pointer_cast<T>(it->second);

    if (node)
      value.push_back(node);
  }

  return value.size() - original_size;
}

template<class T>
bool Configuration::getObject(const std::string &name, std::shared_ptr<T> &ptr) const{

  if( child_objects.count(name) == 0 ){
    return false; }

  std::multimap<std::string, Object*>::iterator it
    = const_cast<Configuration*>(this)->child_objects.find(name);
  T *_value = dynamic_cast<T*>(it->second);

  if (_value == NULL) {
    return false; }

  ptr = _value;
  return true;
}

template<class T>
bool Configuration::getObject(std::shared_ptr<T> &ptr) const {

  Configuration *_this = const_cast<Configuration*>(this);

  for( std::multimap<std::string, std::shared_ptr<Object>>::iterator
         it = _this->child_objects.begin() ;
       it != _this->child_objects.end() ; ++it ){

    std::shared_ptr<T> _value = std::dynamic_pointer_cast<T>(it->second);

    if (_value != NULL) {
      ptr = _value;
      return true;
    }
  }

  return false;
}

template<class T>
void Configuration::setObject(const std::string &name, std::shared_ptr<T> ptr) {
  child_objects.insert(std::pair<std::string, std::shared_ptr<Object>>(name, ptr));
}

template<class T>
bool Configuration::getParam(const std::string &name, T &value) const{

  std::string string_value;
  if (!getParamAsString(name, string_value))
    return false;

  try {

    std::stringstream string_value_stream(string_value);

    T _value;
    string_value_stream >> _value;

    value = _value;
    return true;
  }
  catch(std::exception e){
    GRAMODS_DEBUG_LOG_W("Could not parse '" << string_value << "' as " << typeid(T).name() << "!");
    return false;
  }
}

template<>
inline bool Configuration::getParam(const std::string &name, bool &value) const {

  std::string string_value;

  if (!getParamAsString(name, string_value))
    return false;

  std::transform(string_value.begin(), string_value.end(),
                 string_value.begin(), ::tolower);

  if (string_value == "true") { value = true; return true; }
  if (string_value == "on") { value = true; return true; }
  if (string_value == "1") { value = true; return true; }

  if (string_value == "false") { value = false; return true; }
  if (string_value == "off") { value = false; return true; }
  if (string_value == "0") { value = false; return true; }

  GRAMODS_DEBUG_LOG_W("Could not parse '" << string_value << "' as bool!");
  return false;
}

template<class T>
void Configuration::setParam(const std::string &name, const T &value){

  std::stringstream ss_value;
  ss_value << value;

  parameters[name] = parameter_t(ss_value.str());
}

END_NAMESPACE_GMCONFIG

#endif

