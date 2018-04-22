
#include <gmCore/Configuration.hh>

#include <gmCore/OFactory.hh>

#include <tinyxml2.h>

BEGIN_NAMESPACE_GMCORE;

Configuration::Configuration() {}

Configuration::Configuration(std::string xml) {
  tinyxml2::XMLDocument doc;

  int xml_err = doc.Parse(xml.c_str());
  if (xml_err != 0) throw std::invalid_argument(doc.GetErrorStr1());

  load(&doc);
}

Configuration::Configuration(tinyxml2::XMLNode *node) {
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
      if (name == "type" || name == "name")
        continue;
      std::string value = attr_it->Value();
      setParam(name,value);
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
    
    const char* type_attribute = node_element->Attribute("type");
    std::string type = type_attribute != NULL
      ? std::string(type_attribute) : std::string(node_it->Value());
    
    const char* name_attribute = node_element->Attribute("name");
    std::string name = name_attribute != NULL
      ? std::string(name_attribute) : type;
    
    std::shared_ptr<Object> nn(OFactory::createObject(type));
    if (nn == NULL){
      GM_ERR("Configuration", "Could not create object of type '" << type << "'");
      continue;
    }

    Configuration node_conf(node_it);

    std::vector<std::string> param_names;
    node_conf.getAllParamNames(param_names);

    for (auto param_name : param_names) {
      std::string value;
      bool good = node_conf.getParamAsString(param_name, value);
      assert(good);
      GM_INF("Configuration", name << " -> " << type << "::" << param_name << " = " << value);
      good = OFactory::getOFI(type)->setParamValueFromString(nn.get(), param_name, value);
      if (!good) {
        GM_ERR("Configuration", "no parameter " << param_name << " available in " << type);
        throw std::invalid_argument("no parameter to match xml attribute");
      }
    }

    std::vector<std::string> child_names;
    node_conf.getAllChildNames(child_names);

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

    setObject(name, nn);
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
}

bool Configuration::hasParam(const std::string &name) {
  return parameters.count(name) > 0;
}

size_t Configuration::getAllParamNames(std::vector<std::string> &name) {
  for (auto param : parameters)
    name.push_back(param.first);
  return parameters.size();
}

size_t Configuration::getAllChildNames(std::vector<std::string> &name) {
  for (auto child : child_objects)
    name.push_back(child.first);
  return child_objects.size();
}

void Configuration::parse_param(tinyxml2::XMLElement *element){
  
  const char* name_attribute = element->Attribute("name");
  if (name_attribute == NULL) { return; }
  std::string name = name_attribute;
  
  const char* value_attribute = element->Attribute("value");
  if (value_attribute == NULL) { return; }
  std::string value = value_attribute;
  
  if (parameters.count(name) != 0) {
    GM_WRN("Configuration", "Cannot set parameter " << name << " to " << value
           << ", already set to " << parameters[name].value << "!");
    return;
  }
  
  parameters[name] = parameter_t(value);
  
  GM_INF("Configuration", "Parsed param: " << name << " = " << value);
}

bool Configuration::getParamAsString(const std::string &name, std::string &value) const {
  if (parameters.count(name) == 0) {
    GM_INF("Configuration", "Could not find " << name);
    return false;
  }
  
  Configuration * _this = const_cast<Configuration*>(this);
  
  value = parameters.find(name)->second.value;
  _this->parameters.find(name)->second.checked = true;
  GM_INF("Configuration", "Read " << name << " = " << value);
  return true;
}

END_NAMESPACE_GMCORE;
