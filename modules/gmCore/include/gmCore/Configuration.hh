
#ifndef GRAMODS_CORE_CONFIGURATION
#define GRAMODS_CORE_CONFIGURATION

#include <gmCore/config.hh>
#include <gmCore/Console.hh>

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <memory>

namespace tinyxml2 {
  struct XMLNode;
  struct XMLElement;
}

BEGIN_NAMESPACE_GMCORE;

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

   In XML every node accepts attributes AS, for specifying container,
   DEF, for specifying handle, and USE, for reusing node with
   specified handle.
*/
class Configuration {

public:

  /**
      Creates an empty configuration.
   */
  Configuration();

  /**
     Consumes arguments --config <file>, --xml <string> and --param
     <identifier>=<value> from the provided command line. Either
     --config or --xml is required, upon which the specified file or
     XML string will be parsed, or std::invalid_argument will be
     thrown. Command line argument --param <identifier>=<value> will
     also override configuration file parameters. For example --param
     head.connectionString=WAND@localhost
  */
  Configuration(int &argc, char *argv[]);

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
  bool hasParam(std::string name);

  /**
     Adds all currently available parameter names to the specified
     vector and returns the count.
   */
  std::size_t getAllParamNames(std::vector<std::string> &name);

  /**
     Adds all currently available object names to the specified vector
     and returns the count. Names used for multiple objects will only
     appear once and be counted once.
   */
  std::size_t getAllObjectNames(std::vector<std::string> &name);

  /**
     Add a parameter value.
  */
  void addParam(std::string name, std::string value);

  /**
     Retrieves a named parameter as a specified type. If there are
     multiple parameters with the same name, there is no saying which
     will be retrieved with this function.

     If the value type is not string, then this function will use
     stringstream to parse most types but will need specializations to
     parse non-standard types. If the parameter is not found, the
     value argument will not be changed.

     @param[in] name name of the parameter to retrieve
     @param[out] value the parameter value, if set
     @return false if the parameter is not set or if the value cannot
     be casted to the specified type.
   */
  template<class T>
  bool getParam(std::string name, T &value) const;

  /**
     Retrieves all parameters with a specified name.

     If the value type is not string, then this function will use
     stringstream to parse most types but will need specializations to
     parse non-standard types.
  */
  template<class T>
  std::size_t getAllParams(std::string name, std::vector<T> &value) const;

  /**
     Retrieve an object by its name. If there are multiple objects of
     the same type, there is no saying which will be retrieved with
     this function.

     @param[in] name name of the parameter to retrieve
     @param[out] ptr a pointer to the object, if set
     @return false if no object was found by that name or if it could
     not be casted to the specified type.
   */
  template<class T>
  bool getObject(std::string name, std::shared_ptr<T> &ptr) const;

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
  void addObject(std::string name, std::shared_ptr<T> ptr);

  /**
     Set an object.
   */
  template<class T>
  void addObject(std::shared_ptr<T> ptr){ addObject(typeid(T).name(), ptr); }

  /**
     Retrieve all objects of a specified type. This function will
     append the retrieved objects to the specified list. Use
     Object as type to retrieve all objects.

     @param[out] value a vector of pointer to fill with objects
     @return the number of objects added to the list.
   */
  template<class T>
  std::size_t getAllObjects(std::vector<std::shared_ptr<T>> &value) const;

  /**
     Retrieve all objects with the specified name.

     @param[in] name name of the objects to retrieve
     @param[out] ptr a pointer to the object, if set
     @return false if no object was found by that name or if it could
     not be casted to the specified type.
   */
  template<class T>
  std::size_t getAllObjects(std::string name, std::vector<std::shared_ptr<T>> &ptr) const;

private:

  typedef std::map<std::string, std::shared_ptr<Object>> def_list;
  typedef std::map<std::string, std::string> override_list;

  void load(tinyxml2::XMLNode *node);

  struct parameter_t {
    parameter_t(std::string value = "")
      : checked(false), value(value) {}
    std::string value;
    bool checked;
  };

  typedef std::vector<std::pair<std::string, parameter_t>> parameter_list;
  typedef std::map<std::string, parameter_t> overrides_list;
  typedef std::vector<std::pair<std::string, std::shared_ptr<Object>>> child_object_list;

  child_object_list child_objects;
  parameter_list parameters;

  std::shared_ptr<def_list> def_objects;
  std::string param_path;
  std::shared_ptr<overrides_list> parameter_overrides;
  bool warn_unused_overrides;

  void parse_param(tinyxml2::XMLElement *element);

  /**
     Read the XML data, create objects as specified by the XML data
     and configure the objects.
   */
  Configuration(tinyxml2::XMLNode *node,
                std::shared_ptr<def_list> defs,
                std::string param_path,
                std::shared_ptr<overrides_list> overrides);
};


template<class T>
std::size_t Configuration::getAllObjects(std::vector<std::shared_ptr<T>> &value) const {

  Configuration *_this = const_cast<Configuration*>(this);

  auto original_size = value.size();

  for( child_object_list::iterator it = _this->child_objects.begin() ;
       it != _this->child_objects.end() ; ++it ){

    std::shared_ptr<T> node = std::dynamic_pointer_cast<T>(it->second);

    if (node)
      value.push_back(node);
  }

  return value.size() - original_size;
}

template<class T>
std::size_t Configuration::getAllObjects(std::string name,
                                         std::vector<std::shared_ptr<T>> &value) const {

  Configuration *_this = const_cast<Configuration*>(this);

  auto original_size = value.size();

  for( child_object_list::iterator it = _this->child_objects.begin() ;
       it != _this->child_objects.end() ; ++it ){

    if (it->first != name)
      continue;

    std::shared_ptr<T> node = std::dynamic_pointer_cast<T>(it->second);

    if (node)
      value.push_back(node);
  }

  return value.size() - original_size;
}

template<class T>
bool Configuration::getObject(std::string name, std::shared_ptr<T> &ptr) const {

  Configuration *_this = const_cast<Configuration*>(this);

  auto it = std::find_if(child_objects.begin(),
                         child_objects.end(),
                         [name](const std::pair<std::string, std::shared_ptr<Object>> &pair) {
                           return pair.first == name;
                         });
  if (it == child_objects.end())
    return false;

  std::shared_ptr<T> _value = std::dynamic_pointer_cast<T>(it->second);

  if (!_value) {
    return false; }

  ptr = _value;
  return true;
}

template<class T>
bool Configuration::getObject(std::shared_ptr<T> &ptr) const {

  Configuration *_this = const_cast<Configuration*>(this);

  for (child_object_list::iterator it = _this->child_objects.begin() ;
       it != _this->child_objects.end() ; ++it) {

    std::shared_ptr<T> _value = std::dynamic_pointer_cast<T>(it->second);

    if (_value != NULL) {
      ptr = _value;
      return true;
    }
  }

  return false;
}

template<class T>
void Configuration::addObject(std::string name, std::shared_ptr<T> ptr) {
  child_objects.push_back(std::pair<std::string, std::shared_ptr<Object>>(name, ptr));
}

template<class T>
bool Configuration::getParam(std::string name, T &value) const{

  std::string string_value;
  if (!getParam(name, string_value))
    return false;

  try {

    std::stringstream string_value_stream(string_value);

    T _value;
    string_value_stream >> _value;

    value = _value;
    return true;
  }
  catch(std::exception e){
    GM_WRN("Configuration", "Could not parse '" << string_value << "' as " << typeid(T).name() << "!");
    return false;
  }
}

template <>
inline bool Configuration::getParam(std::string name, std::string &value) const {

  Configuration * _this = const_cast<Configuration*>(this);

  auto it = std::find_if(_this->parameters.begin(),
                         _this->parameters.end(),
                         [name](std::pair<std::string, parameter_t> pair) {
                           return pair.first == name;
                         });
  if (it == _this->parameters.end()) {
    GM_INF("Configuration", "Could not find " << name);
    return false;
  }

  value = it->second.value;
  it->second.checked = true;
  GM_INF("Configuration", "Read " << name << " = " << value);
  return true;
}

template<>
inline bool Configuration::getParam(std::string name, bool &value) const {

  std::string string_value;

  if (!getParam(name, string_value))
    return false;

  std::transform(string_value.begin(), string_value.end(),
                 string_value.begin(), ::tolower);

  if (string_value == "true") { value = true; return true; }
  if (string_value == "on") { value = true; return true; }
  if (string_value == "1") { value = true; return true; }

  if (string_value == "false") { value = false; return true; }
  if (string_value == "off") { value = false; return true; }
  if (string_value == "0") { value = false; return true; }

  GM_WRN("Configuration", "Could not parse '" << string_value << "' as bool!");
  return false;
}

template<>
inline std::size_t Configuration::getAllParams(std::string name, std::vector<std::string> &value) const {

  Configuration * _this = const_cast<Configuration*>(this);
  std::size_t original_size = value.size();

  for (auto &param : _this->parameters)
    if (param.first == name) {
      value.push_back(param.second.value);
      param.second.checked = true;
      GM_INF("Configuration", "Read " << name << " = " << param.second.value);
    }

  return value.size() - original_size;
}

template<>
inline std::size_t Configuration::getAllParams(std::string name, std::vector<bool> &value) const {
   std::vector<std::string> values;
  getAllParams(name, values);

  std::size_t original_size = value.size();
  for (auto string_value : values)
    try {
      std::transform(string_value.begin(), string_value.end(),
                     string_value.begin(), ::tolower);

      if (string_value == "true") { value.push_back(true); continue; }
      if (string_value == "on") { value.push_back(true); continue; }
      if (string_value == "1") { value.push_back(true); continue; }

      if (string_value == "false") { value.push_back(false); continue; }
      if (string_value == "off") { value.push_back(false); continue; }
      if (string_value == "0") { value.push_back(false); continue; }

      GM_WRN("Configuration", "Could not parse '" << string_value << "' as bool!");
    }
    catch(std::exception e){
      GM_WRN("Configuration", "Could not parse '" << string_value << "' as bool!");
    }
  return value.size() - original_size;
}

template<class T>
inline std::size_t Configuration::getAllParams(std::string name, std::vector<T> &value) const {
  std::vector<std::string> values;
  getAllParams(name, values);

  std::size_t original_size = value.size();
  for (auto string_value : values)
    try {
      T _value;
      std::stringstream(string_value) >> _value;

      value.push_back(_value);
    }
    catch(std::exception e){
      GM_WRN("Configuration", "Could not parse '" << string_value << "' as bool!");
    }
  return value.size() - original_size;
}

END_NAMESPACE_GMCORE;

#endif

