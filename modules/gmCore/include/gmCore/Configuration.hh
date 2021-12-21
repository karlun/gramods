
#ifndef GRAMODS_CORE_CONFIGURATION
#define GRAMODS_CORE_CONFIGURATION

#include <gmCore/config.hh>
#include <gmCore/Console.hh>

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <memory>
#include <cctype>

#ifdef gramods_ENABLE_TinyXML2

#include <tinyxml2.h>

BEGIN_NAMESPACE_GMCORE;

class Object;

/**
   A wrapper for the XML parser that also creates and configures the
   system objects and holds temporary references to them for easy
   retrieval. This unit will build an internal tree structure of
   objects that can be easily retrieved by name or by type, and lists
   of parameters for each object.

   Typical usage:

   ~~~~~{.cpp}
   int width = DEFAULT_WIDTH;
   config->getParam("width", width);
   ~~~~~

   In XML every node accepts attributes KEY, for specifying container,
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
     Consumes arguments --config \<file\>, --xml \<string\> and
     --param \<identifier\>=\<value\> from the provided command
     line. Either --config or --xml is required, upon which the
     specified file or XML string will be parsed, or
     gmCore::InvalidArgument will be thrown. Command line argument
     --param \<identifier\>=\<value\> will also override configuration
     file parameters. For example --param
     head.connectionString=WAND\@localhost
  */
  Configuration(int &argc, char *argv[],
                std::vector<std::string> *error_list = nullptr);

  /**
     Loads an XML string, create objects as specified by the XML data
     and configure the objects.
   */
  Configuration(std::string config,
                std::vector<std::string> *error_list = nullptr);

  /**
     Read the XML data, create objects as specified by the XML data
     and configure the objects.
   */
  Configuration(tinyxml2::XMLNode *node,
                std::vector<std::string> *error_list = nullptr);

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
     Adds all currently available object's keys to the specified vector
     and returns the count. Keys used for multiple objects will only
     appear once and be counted once.
   */
  std::size_t getAllObjectKeys(std::vector<std::string> &name);

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
     Retrieve an object by its key. If there are multiple objects of
     the same type, then the first object will be retrieved with this
     function.

     @param[in] key The key of the object to retrieve

     @param[out] ptr A pointer to the object, if set

     @return False if no object was found by that name or if it could
     not be casted to the specified type.
   */
  template<class T>
  bool getObjectByKey(std::string key, std::shared_ptr<T> &ptr) const;

  /**
     Retrieve an object by its DEF name.

     @param[in] def The DEF name of the object to retrieve

     @param[out] ptr A pointer to the object, if set

     @return False if no object was found by that name or if it could
     not be casted to the specified type.
   */
  template<class T>
  bool getObjectByDef(std::string def, std::shared_ptr<T> &ptr) const;

  /**
     Retrieve an object by its type. If there are multiple objects of
     the same type, then the first object will be retrieved with this
     function.

     @param[out] ptr A pointer to the retrieved object, if set

     @return False if no object was found by that name or if it could
     not be casted to the specified type.
   */
  template<class T>
  bool getObject(std::shared_ptr<T> &ptr) const;

  /**
     Set an object.
   */
  template<class T>
  void addObject(std::shared_ptr<T> ptr, std::string key, std::string def);

  /**
     Retrieve all objects of a specified type. This function will
     append the retrieved objects to the specified list. Use
     gmCore::Object as type to retrieve all objects.

     @param[out] value a vector of pointer to fill with objects

     @return the number of objects added to the list.
   */
  template<class T>
  std::size_t getAllObjects(std::vector<std::shared_ptr<T>> &value) const;

  /**
     Retrieve all objects with the specified key.

     @param[in] key The key of the objects to retrieve

     @param[out] ptr A pointer to the retrieved object, if set

     @return False if no object was found by that name or if it could
     not be casted to the specified type.
   */
  template<class T>
  std::size_t getAllObjectsByKey(std::string key, std::vector<std::shared_ptr<T>> &ptr) const;

private:

  typedef std::map<std::string, std::shared_ptr<Object>> def_list;

  void load(tinyxml2::XMLNode *node,
            std::vector<std::string> *error_list = nullptr);

  struct parameter_t {
    parameter_t(std::string value = "")
      : value(value), checked(false) {}
    std::string value;
    bool checked;
  };

  typedef std::vector<std::pair<std::string, parameter_t>> parameter_list;
  typedef std::map<std::string, std::shared_ptr<parameter_t>> overrides_list;
  struct child_t {
    std::shared_ptr<Object> object;
    std::string key;
    std::string def;
  };
  typedef std::vector<child_t> child_object_list;

  child_object_list child_objects;
  parameter_list parameters;

  std::shared_ptr<def_list> def_objects;
  overrides_list parameter_overrides;
  bool warn_unused_overrides;

  bool parse_if(tinyxml2::XMLElement *element,
                std::vector<std::string> *error_list);
  void parse_param(tinyxml2::XMLElement *element,
                   std::vector<std::string> *error_list);

  overrides_list propagateOverrides(const overrides_list &,
                                    std::vector<std::string>);

  /**
     Read the XML data, create objects as specified by the XML data
     and configure the objects.
   */
  Configuration(tinyxml2::XMLNode *node,
                std::shared_ptr<def_list> defs,
                overrides_list overrides,
                std::vector<std::string> *error_list = nullptr);
};


template<class T>
std::size_t Configuration::getAllObjects(std::vector<std::shared_ptr<T>> &value) const {

  Configuration *_this = const_cast<Configuration*>(this);

  auto original_size = value.size();

  for (auto &child : _this->child_objects) {
    std::shared_ptr<T> node = std::dynamic_pointer_cast<T>(child.object);
    if (node) value.push_back(node);
  }

  return value.size() - original_size;
}

template<class T>
std::size_t Configuration::getAllObjectsByKey(
    std::string key, std::vector<std::shared_ptr<T>> &value) const {

  Configuration *_this = const_cast<Configuration*>(this);

  auto original_size = value.size();

  for (auto &child : _this->child_objects) {

    if (child.key != key) continue;

    std::shared_ptr<T> node = std::dynamic_pointer_cast<T>(child.object);
    if (node) value.push_back(node);
  }

  return value.size() - original_size;
}

template<class T>
bool Configuration::getObjectByKey(std::string key, std::shared_ptr<T> &ptr) const {

  auto it = std::find_if(child_objects.begin(),
                         child_objects.end(),
                         [key](const child_t &child) {
                           return key == child.key;
                         });
  if (it == child_objects.end()) return false;

  std::shared_ptr<T> _value = std::dynamic_pointer_cast<T>(it->object);
  if (!_value) return false;

  ptr = _value;
  return true;
}

template<class T>
bool Configuration::getObjectByDef(std::string def, std::shared_ptr<T> &ptr) const {

  if (def_objects->count(def) == 0) return false;

  std::shared_ptr<T> _value = std::dynamic_pointer_cast<T>((*def_objects)[def]);
  if (!_value) return false;

  ptr = _value;
  return true;
}

template<class T>
bool Configuration::getObject(std::shared_ptr<T> &ptr) const {

  Configuration *_this = const_cast<Configuration*>(this);

  for (auto &child : _this->child_objects) {

    std::shared_ptr<T> _value = std::dynamic_pointer_cast<T>(child.object);

    if (_value == nullptr) continue;

    ptr = _value;
    return true;
  }

  return false;
}

template<class T>
void Configuration::addObject(std::shared_ptr<T> ptr,
                              std::string key,
                              std::string def) {
  child_objects.push_back(child_t({ptr, key, def}));
}

template<class T>
bool Configuration::getParam(std::string name, T &value) const {

  std::string string_value;
  if (!getParam(name, string_value))
    return false;

  try {

    std::stringstream string_value_stream(string_value);

    T _value;
    string_value_stream >> _value;

    if (!string_value_stream) {
      GM_WRN("Configuration", "While getting '" << name << "', could not parse '" << string_value << "' as " << typeid(T).name() << "!");
      return false;
    }

    value = _value;
    return true;
  }
  catch (std::exception){
    GM_WRN("Configuration", "While getting '" << name << "', could not parse '" << string_value << "' as " << typeid(T).name() << "!");
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
    GM_DBG1("Configuration", "Could not find " << name);
    return false;
  }

  value = it->second.value;
  it->second.checked = true;
  GM_DBG1("Configuration", "Read " << name << " = " << value);
  return true;
}

template<>
inline bool Configuration::getParam(std::string name, bool &value) const {

  std::string string_value;

  if (!getParam(name, string_value))
    return false;

  std::transform(string_value.begin(), string_value.end(), string_value.begin(),
                 [](unsigned char c){ return std::tolower(c); });

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
      GM_DBG1("Configuration", "Read " << name << " = " << param.second.value);
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
      std::transform(string_value.begin(), string_value.end(), string_value.begin(),
                     [](unsigned char c){ return std::tolower(c); });

      if (string_value == "true") { value.push_back(true); continue; }
      if (string_value == "on") { value.push_back(true); continue; }
      if (string_value == "1") { value.push_back(true); continue; }

      if (string_value == "false") { value.push_back(false); continue; }
      if (string_value == "off") { value.push_back(false); continue; }
      if (string_value == "0") { value.push_back(false); continue; }

      GM_WRN("Configuration", "Could not parse '" << string_value << "' as bool!");
    } catch (std::exception &) {
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

      std::stringstream string_value_stream(string_value);

      T _value;
      string_value_stream >> _value;

      if (!string_value_stream) {
      GM_WRN("Configuration", "While getting '" << name << "', could not parse '" << string_value << "' as " << typeid(T).name() << "!");
        return false;
      }

      value.push_back(_value);
    }
    catch (std::exception){
      GM_WRN("Configuration", "While getting '" << name << "', could not parse '" << string_value << "' as " << typeid(T).name() << "!");
    }
  return value.size() - original_size;
}

END_NAMESPACE_GMCORE;

#endif

#endif

