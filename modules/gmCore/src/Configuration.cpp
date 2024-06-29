
#include <gmCore/Configuration.hh>

#ifdef gramods_ENABLE_TinyXML2

#include <gmCore/OFactory.hh>
#include <gmCore/CommandLineParser.hh>
#include <gmCore/InvalidArgument.hh>
#include <gmCore/FileResolver.hh>
#include <gmCore/Stringify.hh>

#include <set>

#include <cstdlib>

BEGIN_NAMESPACE_GMCORE;

Configuration::Configuration()
  : def_objects(std::make_shared<def_list>()),
    warn_unused_overrides(false) {}

Configuration::Configuration(std::string xml,
                             std::vector<std::string> *error_list)
  : def_objects(std::make_shared<def_list>()),
    warn_unused_overrides(false) {

  tinyxml2::XMLDocument doc;

  int xml_err = doc.Parse(xml.c_str());
  if (xml_err != 0) throw gmCore::InvalidArgument(doc.ErrorStr());

  load(&doc, error_list);
}

Configuration::Configuration(int &argc, char *argv[],
                             std::vector<std::string> *error_list)
  : def_objects(std::make_shared<def_list>()),
    warn_unused_overrides(true) {

  std::vector<std::string> configs;
  std::vector<std::string> xmls;

  CommandLineParser cmd(argc, argv);

  while (cmd.hasMoreArguments()) {

    std::string value = cmd.getNextArgument();

    if (value == "--config") {

      if (!cmd.hasMoreArguments())
        throw gmCore::InvalidArgument("--config missing value");

      value = cmd.getNextArgument();
      cmd.consumeLast(2);

      configs.push_back(value);

    } else if (value == "--xml") {

      if (!cmd.hasMoreArguments())
        throw gmCore::InvalidArgument("--xml missing value");

      value = cmd.getNextArgument();
      cmd.consumeLast(2);

      xmls.push_back(value);

    } else if (value == "--param") {
 
      if (!cmd.hasMoreArguments())
        throw gmCore::InvalidArgument("--param missing value");

      value = cmd.getNextArgument();
      cmd.consumeLast(2);

      std::size_t sep_pos = value.find("=");
      if (sep_pos == std::string::npos || sep_pos + 1 == value.size())
        throw gmCore::InvalidArgument("--param value is not in form node.param=value");

      std::string name = value.substr(0, sep_pos);
      value = value.substr(sep_pos + 1);

      parameter_overrides[name] = std::make_shared<parameter_t>(value);
    }
  }

  if (configs.empty() && xmls.empty())
    throw gmCore::InvalidArgument("Either --config or --xml must be specified at least once");

  for (auto config : configs) {

    std::filesystem::path file = FileResolver::getDefault()->resolve(
        config, FileResolver::Check::ReadableFile);

    tinyxml2::XMLDocument doc;

    int xml_err = doc.LoadFile(file.u8string().c_str());
    if (xml_err != 0) {
      GM_ERR("Configuration", doc.ErrorStr());
      throw gmCore::InvalidArgument(doc.ErrorStr());
    }

    load(&doc, error_list);
  }

  for (auto xml : xmls) {

    tinyxml2::XMLDocument doc;

    int xml_err = doc.Parse(xml.c_str());
    if (xml_err != 0) {
      GM_ERR("Configuration", doc.ErrorStr());
      throw gmCore::InvalidArgument(doc.ErrorStr());
    }

    load(&doc, error_list);
  }
}

Configuration::Configuration(tinyxml2::XMLNode *node,
                             std::shared_ptr<def_list> defs,
                             overrides_list overrides,
                             std::vector<std::string> *error_list)
  : def_objects(defs),
    parameter_overrides(overrides),
    warn_unused_overrides(false) {
  load(node, error_list);
}

void Configuration::load(tinyxml2::XMLNode *node,
                         std::vector<std::string> *error_list) {

  tinyxml2::XMLDocument *doc = node->ToDocument();
  if (doc != NULL)
    node = doc->RootElement();

  tinyxml2::XMLElement *node_element = node->ToElement();
  if (node_element != NULL &&
      strcmp(node_element->Value(), "if") != 0) {
    for( const tinyxml2::XMLAttribute *attr_it = node->ToElement()->FirstAttribute() ;
         attr_it != NULL ; attr_it = attr_it->Next()) {
      std::string name = attr_it->Name();
      if (name == "KEY" || name == "DEF" || name == "USE")
        continue;
      std::string value = attr_it->Value();
      addParam(name,value);
    }
  }

  for( tinyxml2::XMLNode* node_it = node->FirstChild() ;
       node_it != NULL ; node_it = node_it->NextSibling()) {
    
    tinyxml2::XMLElement *node_element = node_it->ToElement();
    if (node_element == NULL) {
      continue;
    }

    if (strcmp(node_it->Value(), "if") == 0) {
      if (parse_if(node_element, error_list))
        load(node_it);
      continue;
    }

    if (strcmp(node_it->Value(), "param") == 0) {
      parse_param(node_element, error_list);
      continue;
    }
    
    std::string type = std::string(node_it->Value());
    
    const char* key_attribute = node_element->Attribute("KEY");
    std::string KEY = key_attribute != NULL
      ? std::string(key_attribute) : "";

    const char* use_attribute = node_element->Attribute("USE");
    if (use_attribute != NULL) {

      std::string USE = std::string(use_attribute);

      if (def_objects->count(USE) == 0) {
        GM_WRN("Configuration", "no DEF to match USE " << USE << " in " << type);
        if (error_list) error_list->push_back(GM_STR("no DEF to match USE " << USE << " in " << type));
        else throw gmCore::InvalidArgument(GM_STR("no DEF to match USE " << USE << " in " << type));
        continue;
      }

      if (KEY.empty()) KEY = (*def_objects)[USE]->getDefaultKey();
      addObject((*def_objects)[USE], KEY, USE);

      continue;
    }

    std::shared_ptr<Object> nn(OFactory::createObject(type));
    if (nn == NULL){
      GM_WRN("Configuration", "Could not create object of type '" << type << "'");
      if (error_list) error_list->push_back(GM_STR("Could not create object of type '" << type << "'"));
      else throw gmCore::InvalidArgument(GM_STR("Could not create object of type '" << type << "'"));
      continue;
    }

    if (KEY.empty()) KEY = nn->getDefaultKey();
    std::vector<std::string> alias({KEY, type});

    const char* def_attribute = node_element->Attribute("DEF");
    std::string DEF = def_attribute != NULL ? std::string(def_attribute) : "";

    if (!DEF.empty()) {
      if (def_objects->count(DEF) != 0) {
        GM_WRN("Configuration", "DEF " << DEF << " has multiple associations");
        if (error_list) error_list->push_back(GM_STR("DEF " << DEF << " has multiple associations"));
        else throw gmCore::InvalidArgument(GM_STR("DEF " << DEF << " has multiple associations"));
      }
      (*def_objects)[DEF] = nn;
      alias.push_back(DEF);
    }

    overrides_list node_overrides =
        propagateOverrides(parameter_overrides, alias);
    Configuration node_conf(node_it, def_objects, node_overrides);

    std::vector<std::string> param_names;
    node_conf.getAllParamNames(param_names);

    for (auto param_name : param_names) {

      GM_DBG2("Configuration", "Processing parameter " << param_name);

      std::vector<std::string> values;
      node_conf.getAllParams(param_name, values);

      if (node_overrides.find(param_name) == node_overrides.end()) {

        for (std::string value : values) {

          GM_DBG2("Configuration", KEY << " -> " <<
                  type << "::" << param_name << " = " << value);

          try {
            bool good = OFactory::getOFI(type)->
              setParamValueFromString(nn.get(), param_name, value);
            if (!good) {
              GM_WRN("Configuration", "no parameter " << param_name << " available in " << type);
              if (error_list) error_list->push_back(GM_STR("no parameter " << param_name << " available in " << type));
              else throw gmCore::InvalidArgument(GM_STR("no parameter " << param_name << " available in " << type));
            }
          } catch (const gmCore::InvalidArgument &e) {
            GM_WRN("Configuration", "While setting parameter: " << e.what);
            if (error_list) error_list->push_back(e.what);
            else throw e;
          }
        }

      } else {

        std::string value = node_overrides[param_name]->value;
        node_overrides[param_name]->checked = true;
        GM_DBG2("Configuration",
                KEY << " -> " << type << "::" << param_name << " = " << value
                    << " (overridden)");

        try {
          bool good = OFactory::getOFI(type)->
            setParamValueFromString(nn.get(), param_name, value);
          if (!good) {
            GM_WRN("Configuration", "no parameter " << param_name << " available in " << type);
            if (error_list) error_list->push_back(GM_STR("no parameter " << param_name << " available in " << type));
            else throw gmCore::InvalidArgument(GM_STR("no parameter " << param_name << " available in " << type));
          }
        } catch (const gmCore::InvalidArgument &e) {
          GM_WRN("Configuration", e.what);
          if (error_list) error_list->push_back(e.what);
          else throw e;
        }
      }
    }

    std::vector<std::string> child_keys;
    node_conf.getAllObjectKeys(child_keys);

    for (auto child_key : child_keys) {
      std::vector<std::shared_ptr<Object>> ptrs;
      node_conf.getAllObjectsByKey(child_key, ptrs);
      assert(ptrs.size() > 0);
      for (auto ptr : ptrs) {
        GM_DBG2("Configuration", KEY << " -> " << type << "::" << child_key << " = ptr");
        try {
          bool good = OFactory::getOFI(type)->setPointerValue(nn.get(), child_key, ptr);
          if (!good) {
            GM_WRN("Configuration", "no pointer '" << child_key << "' to match instance of '" << typeid(*ptr).name() << "' available in " << type);
            if (error_list) error_list->push_back(GM_STR("no pointer '" << child_key << "' to match instance of '" << typeid(*ptr).name() << "' available in " << type));
            else throw gmCore::InvalidArgument(GM_STR("no pointer '" << child_key << "' to match instance of '" << typeid(*ptr).name() << "' available in " << type));
          }
        } catch (const gmCore::InvalidArgument &e) {
          GM_WRN("Configuration", e.what);
          if (error_list) error_list->push_back(e.what);
          else throw e;
        }
      }
    }

    nn->initialize();
    if (!nn->isInitialized()) {
      GM_WRN("Configuration", "Could not initialize instance of " << type);
      if (error_list) error_list->push_back(GM_STR("Could not initialize instance of " << type));
      else throw gmCore::InvalidArgument(GM_STR("Could not initialize instance of " << type));
      continue;
    }

    addObject(nn, KEY, DEF);
  }
}

Configuration::~Configuration(){
  for( parameter_list::iterator it = parameters.begin() ;
       it != parameters.end() ; ++it) {
    if (! it->second.checked) {
      GM_WRN("Configuration", "Parameter '" << it->first << "', "
             << "set to '" << it->second.value << "', "
             << "has not been used!");
    }
  }

  if (warn_unused_overrides)
    for (auto param : parameter_overrides)
      if (!param.second->checked)
        GM_WRN("Configuration", "Override '" << param.first << "', "
               << "set to '" << param.second->value << "', "
               << "has not been used!");

  // Delete in reverse order; at least ImportLibrary must be deleted
  // after the Objects imported with it, or their virtual destructors
  // won't be available when being deleted.
  for (auto ptr = child_objects.rbegin(); ptr != child_objects.rend(); ptr++)
    ptr->object.reset();
}

bool Configuration::hasParam(std::string name) {
  auto it = std::find_if(parameters.begin(),
                         parameters.end(),
                         [name](std::pair<std::string, parameter_t> &pair) {
                           return pair.first == name;
                         });
  return it != parameters.end();
}

void Configuration::addParam(std::string name, std::string value) {
  GM_DBG2("Configuration", "addParam(" << name << ", " << value << ")");
  parameters.push_back(std::pair<std::string, parameter_t>(name, parameter_t(value)));
}

size_t Configuration::getAllParamNames(std::vector<std::string> &name) {
  std::vector<std::string> new_names;
  for (auto param : parameters)
    if (std::find(new_names.begin(), new_names.end(), param.first) == new_names.end())
      new_names.push_back(param.first);

  name.insert(name.end(), new_names.begin(), new_names.end());

  return new_names.size();
}

size_t Configuration::getAllObjectKeys(std::vector<std::string> &name) {
  std::set<std::string> new_names;
  for (auto child : child_objects) new_names.insert(child.key);

  name.insert(name.end(), new_names.begin(), new_names.end());
  return new_names.size();
}

bool Configuration::parse_if(tinyxml2::XMLElement *element,
                             std::vector<std::string> *error_list) {

  const char* variable_attribute = element->Attribute("variable");
  if (variable_attribute == NULL) {
    GM_WRN("Configuration", "Node \"if\" is missing expected attribute \"variable\"");
    if (error_list) error_list->push_back("Node if is missing expected attribute \"variable\"");
    else throw gmCore::InvalidArgument("Node if is missing expected attribute \"variable\"");
    return false;
  }

  char * variable_value = std::getenv(variable_attribute);
  if (variable_value == NULL) {
    GM_WRN("Configuration", "Environment variable \"" << variable_attribute << "\" not found - cannot compare.");
    if (error_list) error_list->push_back(GM_STR("Environment variable \"" << variable_attribute << "\" not found - cannot compare."));
    else throw gmCore::InvalidArgument(GM_STR("Environment variable \"" << variable_attribute << "\" not found - cannot compare."));
    return false;
  }
  std::string variable = variable_value;

  const char* value_attribute = element->Attribute("value");
  if (value_attribute == NULL) {
    GM_WRN("Configuration", "Node \"if\" is missing expected attribute \"value\"");
    if (error_list) error_list->push_back("Node if is missing expected attribute \"value\"");
    else throw gmCore::InvalidArgument("Node if is missing expected attribute \"value\"");
    return false;
  }
  std::string value = value_attribute;

  return variable == value;
}

void Configuration::parse_param(tinyxml2::XMLElement *element,
                                std::vector<std::string> *error_list){
  
  const char* name_attribute = element->Attribute("name");
  if (name_attribute == NULL) {
    GM_WRN("Configuration", "Node \"param\" is missing expected attribute \"name\"");
    if (error_list) error_list->push_back("Node \"param\" is missing expected attribute \"name\"");
    else throw gmCore::InvalidArgument("Node \"param\" is missing expected attribute \"name\"");
    return;
  }
  
  const char* value_attribute = element->Attribute("value");
  if (value_attribute == NULL) {
    GM_WRN("Configuration", "Node \"param\" is missing expected attribute \"value\"");
    if (error_list) error_list->push_back("Node \"param\" is missing expected attribute \"value\"");
    else throw gmCore::InvalidArgument("Node \"param\" is missing expected attribute \"value\"");
    return;
  }

  std::string name = name_attribute;
  std::string value = value_attribute;

  GM_DBG2("Configuration", "Parsed param: " << name << " = " << value);

  addParam(name, value);
}

Configuration::overrides_list
Configuration::propagateOverrides(const overrides_list &list,
                                  std::vector<std::string> alias) {

  overrides_list newlist;

  if (list.empty() || alias.empty()) return newlist;

  for (auto &item : list) {

    std::string name = item.first;
    auto first_dot = name.find(".");
    if (first_dot == std::string::npos) continue;
    name = name.substr(0, first_dot);

    if (std::find(alias.begin(), alias.end(), name) == alias.end()) continue;
    std::string res = item.first.substr(first_dot + 1);
    newlist[res] = item.second;
  }

  if (!newlist.empty()) {
    GM_DBG2("Configuration",
            "Propagating (" << stringify(newlist) << ") to ("
                            << stringify(alias) << ")");
  } else {
    GM_DBG2("Configuration",
            "Not propagating (" << stringify(list) << ") to ("
                                << stringify(alias) << ")");
  }

  return newlist;
}

END_NAMESPACE_GMCORE;

#endif
