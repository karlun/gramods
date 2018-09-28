
#include <gmCore/Configuration.hh>

#include <gmCore/OFactory.hh>
#include <gmCore/CommandLineParser.hh>

#include <tinyxml2.h>

BEGIN_NAMESPACE_GMCORE;

Configuration::Configuration()
  : def_objects(std::make_shared<def_list>()),
    parameter_overrides(std::make_shared<overrides_list>()),
    warn_unused_overrides(false) {}

Configuration::Configuration(std::string xml)
  : def_objects(std::make_shared<def_list>()),
    parameter_overrides(std::make_shared<overrides_list>()),
    warn_unused_overrides(false) {

  tinyxml2::XMLDocument doc;

  int xml_err = doc.Parse(xml.c_str());
  if (xml_err != 0) throw std::invalid_argument(doc.ErrorStr());

  load(&doc);
}

Configuration::Configuration(int &argc, char *argv[])
  : def_objects(std::make_shared<def_list>()),
    warn_unused_overrides(true),
    parameter_overrides(std::make_shared<overrides_list>()) {

  std::vector<std::string> configs;
  std::vector<std::string> xmls;

  CommandLineParser cmd(argc, argv);

  while (cmd.hasMoreArguments()) {

    std::string value = cmd.getNextArgument();

    if (value == "--config") {

      if (!cmd.hasMoreArguments())
        throw std::invalid_argument("--config missing value");

      value = cmd.getNextArgument();
      bool res = cmd.consumeLast(2);
      assert(res);

      configs.push_back(value);

    } else if (value == "--xml") {

      if (!cmd.hasMoreArguments())
        throw std::invalid_argument("--xml missing value");

      value = cmd.getNextArgument();
      bool res = cmd.consumeLast(2);
      assert(res);

      xmls.push_back(value);

    } else if (value == "--param") {
 
      if (!cmd.hasMoreArguments())
        throw std::invalid_argument("--param missing value");

      value = cmd.getNextArgument();
      bool res = cmd.consumeLast(2);
      assert(res);

      std::size_t sep_pos = value.find("=");
      if (sep_pos == std::string::npos || sep_pos + 1 == value.size())
        throw std::invalid_argument("--param value is not in form node.param=value");

      std::string name = value.substr(0, sep_pos);
      value = value.substr(sep_pos + 1);

      (*parameter_overrides)[name] = parameter_t(value);
    }
  }

  if (configs.empty() && xmls.empty())
    throw std::invalid_argument("Either --config or --xml must be specified at least once");

  for (auto config : configs) {

    tinyxml2::XMLDocument doc;

    int xml_err = doc.LoadFile(config.c_str());
    if (xml_err != 0) {
      GM_ERR("Configuration", doc.ErrorStr());
      throw std::invalid_argument(doc.ErrorStr());
    }

    load(&doc);
  }

  for (auto xml : xmls) {

    tinyxml2::XMLDocument doc;

    int xml_err = doc.Parse(xml.c_str());
    if (xml_err != 0) {
      GM_ERR("Configuration", doc.ErrorStr());
      throw std::invalid_argument(doc.ErrorStr());
    }

    load(&doc);
  }
}

Configuration::Configuration(tinyxml2::XMLNode *node,
                             std::shared_ptr<def_list> defs,
                             std::string param_path,
                             std::shared_ptr<overrides_list> overrides)
  : def_objects(defs),
    param_path(param_path),
    parameter_overrides(overrides),
    warn_unused_overrides(false) {
  load(node);
}

void Configuration::load(tinyxml2::XMLNode *node) {
  tinyxml2::XMLDocument *doc = node->ToDocument();
  if (doc != NULL)
    node = doc->RootElement();

  tinyxml2::XMLElement *node_element = node->ToElement();
  if (node_element != NULL) {
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
    
    if (strcasecmp( node_it->Value(), "param" ) == 0) {
      parse_param(node_element);
      continue;
    }
    
    std::string type = std::string(node_it->Value());
    
    const char* name_attribute = node_element->Attribute("AS");
    std::string name = name_attribute != NULL
      ? std::string(name_attribute) : type;
    
    const char* def_attribute = node_element->Attribute("DEF");
    std::string DEF = def_attribute != NULL
      ? std::string(def_attribute) : "";
    
    const char* use_attribute = node_element->Attribute("USE");
    std::string USE = use_attribute != NULL
      ? std::string(use_attribute) : "";

    if (USE != "") {
      if (def_objects->count(USE) == 0) {
        GM_ERR("Configuration", "no DEF to match USE " << USE << " in " << type);
        throw std::invalid_argument("no DEF to match USE");
      }
      addObject(name, (*def_objects)[USE]);
      continue;
    }

    std::shared_ptr<Object> nn(OFactory::createObject(type));
    if (nn == NULL){
      GM_ERR("Configuration", "Could not create object of type '" << type << "'");
      continue;
    }

    if (DEF != "") {
      if (def_objects->count(USE) != 0) {
        GM_ERR("Configuration", "DEF " << DEF << " has multiple associations");
        throw std::invalid_argument("DEF has multiple associations");
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

      if (parameter_overrides->find(param_path) ==
          parameter_overrides->end()) {

        std::vector<std::string> values;
        size_t count = node_conf.getAllParams(param_name, values);
        assert(count > 0);

        for (std::string value : values) {
          GM_INF("Configuration", name << " -> " <<
                 type << "::" << param_name << " = " << value);
          bool good = OFactory::getOFI(type)->
            setParamValueFromString(nn.get(), param_name, value);
          if (!good) {
            GM_ERR("Configuration", "no parameter " << param_name << " available in " << type);
            throw std::invalid_argument("no parameter to match xml attribute");
          }
        }

      } else {

        std::string value = (*parameter_overrides)[param_path].value;
        (*parameter_overrides)[param_path].checked = true;
        GM_INF("Configuration", name << " -> " <<
               type << "::" << param_name << " = " << value << " (overridden)");

        bool good = OFactory::getOFI(type)->
          setParamValueFromString(nn.get(), param_name, value);
        if (!good) {
          GM_ERR("Configuration", "no parameter " << param_name << " available in " << type);
          throw std::invalid_argument("no parameter to match xml attribute");
        }
      }
    }

    std::vector<std::string> child_names;
    node_conf.getAllObjectNames(child_names);

    for (auto child_name : child_names) {
      std::vector<std::shared_ptr<Object>> ptrs;
      node_conf.getAllObjects(child_name, ptrs);
      assert(ptrs.size() > 0);
      for (auto ptr : ptrs) {
        GM_INF("Configuration", name << " -> " << type << "::" << child_name << " = ptr");
        bool good = OFactory::getOFI(type)->setPointerValue(nn.get(), child_name, ptr);
        if (!good) {
          GM_ERR("Configuration", "no pointer " << child_name << " available in " << type);
          throw std::invalid_argument("no parameter to match xml attribute");
        }
      }
    }

    nn->initialize();
    if (!nn->isInitialized())
      GM_ERR("Configuration", "Could not initialize instance of " << type);
    else
      addObject(name, nn);
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
  return parameters.count(name) > 0;
}

void Configuration::addParam(std::string name, std::string value){
  parameters.insert(std::pair<std::string, parameter_t>(name, parameter_t(value)));
}

size_t Configuration::getAllParamNames(std::vector<std::string> &name) {
  std::vector<std::string> new_names;
  for (auto param : parameters)
    new_names.push_back(param.first);

  std::vector<std::string>::iterator it;
  it = std::unique(new_names.begin(), new_names.end());
  new_names.resize(std::distance(new_names.begin(), it));

  name.insert(name.end(), new_names.begin(), new_names.end());

  return new_names.size();
}

size_t Configuration::getAllObjectNames(std::vector<std::string> &name) {
  std::vector<std::string> new_names;
  for (auto child : child_objects)
    new_names.push_back(child.first);

  std::vector<std::string>::iterator it;
  it = std::unique(new_names.begin(), new_names.end());
  new_names.resize(std::distance(new_names.begin(), it));

  name.insert(name.end(), new_names.begin(), new_names.end());

  return new_names.size();
}

void Configuration::parse_param(tinyxml2::XMLElement *element){
  
  const char* name_attribute = element->Attribute("name");
  if (name_attribute == NULL) { return; }
  std::string name = name_attribute;
  
  const char* value_attribute = element->Attribute("value");
  if (value_attribute == NULL) { return; }
  std::string value = value_attribute;

  GM_INF("Configuration", "Parsed param: " << name << " = " << value);

  addParam(name, value);
}

END_NAMESPACE_GMCORE;
