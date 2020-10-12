
#include <gmCore/Configuration.hh>

#ifdef gramods_ENABLE_TinyXML2

#include <gmCore/OFactory.hh>
#include <gmCore/CommandLineParser.hh>
#include <gmCore/InvalidArgument.hh>
#include <gmCore/FileResolver.hh>

#include <stdlib.h>

BEGIN_NAMESPACE_GMCORE;

Configuration::Configuration()
  : def_objects(std::make_shared<def_list>()),
    parameter_overrides(std::make_shared<overrides_list>()),
    warn_unused_overrides(false) {}

Configuration::Configuration(std::string xml,
                             std::vector<std::string> *error_list)
  : def_objects(std::make_shared<def_list>()),
    parameter_overrides(std::make_shared<overrides_list>()),
    warn_unused_overrides(false) {

  tinyxml2::XMLDocument doc;

  int xml_err = doc.Parse(xml.c_str());
  if (xml_err != 0) throw gmCore::InvalidArgument(doc.ErrorStr());

  load(&doc, error_list);
}

Configuration::Configuration(int &argc, char *argv[],
                             std::vector<std::string> *error_list)
  : def_objects(std::make_shared<def_list>()),
    parameter_overrides(std::make_shared<overrides_list>()),
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

      (*parameter_overrides)[name] = parameter_t(value);
    }
  }

  if (configs.empty() && xmls.empty())
    throw gmCore::InvalidArgument("Either --config or --xml must be specified at least once");

  for (auto config : configs) {

    std::filesystem::path file =
      FileResolver::getDefault()->resolve(config);

    tinyxml2::XMLDocument doc;

    int xml_err = doc.LoadFile(file.c_str());
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
                             std::string param_path,
                             std::shared_ptr<overrides_list> overrides,
                             std::vector<std::string> *error_list)
  : def_objects(defs),
    param_path(param_path),
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
      if (name == "AS" || name == "DEF" || name == "USE")
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
    
    const char* as_attribute = node_element->Attribute("AS");
    std::string name = as_attribute != NULL
      ? std::string(as_attribute) : type;
    
    const char* def_attribute = node_element->Attribute("DEF");
    std::string DEF = def_attribute != NULL
      ? std::string(def_attribute) : "";
    
    const char* use_attribute = node_element->Attribute("USE");
    std::string USE = use_attribute != NULL
      ? std::string(use_attribute) : "";

    if (USE != "") {

      if (def_objects->count(USE) == 0) {
        GM_WRN("Configuration", "no DEF to match USE " << USE << " in " << type);
        if (error_list) error_list->push_back(GM_STR("no DEF to match USE " << USE << " in " << type));
        else throw gmCore::InvalidArgument(GM_STR("no DEF to match USE " << USE << " in " << type));
        continue;
      }

      if (as_attribute != NULL)
        addObject(std::string(as_attribute), (*def_objects)[USE]);
      else
        addObject((*def_objects)[USE]->getDefaultKey(), (*def_objects)[USE]);

      continue;
    }

    std::shared_ptr<Object> nn(OFactory::createObject(type));
    if (nn == NULL){
      GM_WRN("Configuration", "Could not create object of type '" << type << "'");
      if (error_list) error_list->push_back(GM_STR("Could not create object of type '" << type << "'"));
      else throw gmCore::InvalidArgument(GM_STR("Could not create object of type '" << type << "'"));
      continue;
    }

    if (DEF != "") {
      if (def_objects->count(USE) != 0) {
        GM_WRN("Configuration", "DEF " << DEF << " has multiple associations");
        if (error_list) error_list->push_back(GM_STR("DEF " << DEF << " has multiple associations"));
        else throw gmCore::InvalidArgument(GM_STR("DEF " << DEF << " has multiple associations"));
      }
      (*def_objects)[DEF] = nn;
    }

    std::string node_path = param_path.size() == 0 ? name : param_path + "." + name;

    Configuration node_conf(node_it, def_objects, node_path, parameter_overrides);

    std::vector<std::string> param_names;
    node_conf.getAllParamNames(param_names);

    for (auto param_name : param_names) {

      std::string param_path = node_path + "." + param_name;
      GM_INF("Configuration", "Checking parameter override for " << param_path);

      std::vector<std::string> values;
      node_conf.getAllParams(param_name, values);

      if (parameter_overrides->find(param_path) ==
          parameter_overrides->end()) {

        for (std::string value : values) {

          GM_INF("Configuration", name << " -> " <<
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

        std::string value = (*parameter_overrides)[param_path].value;
        (*parameter_overrides)[param_path].checked = true;
        GM_INF("Configuration", name << " -> " <<
               type << "::" << param_name << " = " << value << " (overridden)");

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
    node_conf.getAllObjectNames(child_keys);

    for (auto child_key : child_keys) {
      std::vector<std::shared_ptr<Object>> ptrs;
      node_conf.getAllObjects(child_key, ptrs);
      assert(ptrs.size() > 0);
      for (auto ptr : ptrs) {
        GM_INF("Configuration", name << " -> " << type << "::" << child_key << " = ptr");
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

    if (as_attribute != NULL)
      addObject(std::string(as_attribute), nn);
    else
      addObject(nn->getDefaultKey(), nn);
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
    for (auto param : *parameter_overrides)
      if (!param.second.checked)
        GM_WRN("Configuration", "Override '" << param.first << "', "
               << "set to '" << param.second.value << "', "
               << "has not been used!");
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
  GM_VINF("Configuration", "addParam(" << name << ", " << value << ")");
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

size_t Configuration::getAllObjectNames(std::vector<std::string> &name) {
  std::vector<std::string> new_names;
  for (auto child : child_objects)
    if (std::find(new_names.begin(), new_names.end(), child.first) == new_names.end())
      new_names.push_back(child.first);

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

  char * variable_value = getenv(variable_attribute);
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

  GM_INF("Configuration", "Parsed param: " << name << " = " << value);

  addParam(name, value);
}

END_NAMESPACE_GMCORE;

#endif
