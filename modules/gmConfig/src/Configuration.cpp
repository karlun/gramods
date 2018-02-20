
#include <gmConfig/Configuration.hh>

#include <gmConfig/OFactory.hh>

#include <tinyxml2.h>

USING_NAMESPACE_GMCONFIG;

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
      GRAMODS_DEBUG_LOG_E("Could not create object of type '" << type << "'");
      continue;
    }
    
    //nn->configure(Configuration(node_it));
    
    setObject(name, nn);
  }
}

Configuration::~Configuration(){
  for( parameter_list::iterator it = parameters.begin() ;
       it != parameters.end() ; ++it) {
    if (! it->second.checked) {
      GRAMODS_DEBUG_LOG_W("Parameter '" << it->first << "', "
                       << "set to '" << it->second.value << "', "
                       << "has not been used!");
    }
  }
}

void Configuration::parse_param(tinyxml2::XMLElement *element){
  
  const char* name_attribute = element->Attribute("name");
  if (name_attribute == NULL) { return; }
  std::string name = name_attribute;
  
  const char* value_attribute = element->Attribute("value");
  if (value_attribute == NULL) { return; }
  std::string value = value_attribute;
  
  if (parameters.count(name) != 0) {
    GRAMODS_DEBUG_LOG_W("Cannot set parameter " << name << " to " << value
                     << ", already set to " << parameters[name].value << "!");
    return;
  }
  
  parameters[name] = parameter_t(value);
  
  GRAMODS_DEBUG_LOG_I("Parsed param: " << name << " = " << value);
}

bool Configuration::getParamAsString(const std::string &name, std::string &value) const {
  if (parameters.count(name) == 0) {
    GRAMODS_DEBUG_LOG_I("Could not find " << name);
    return false;
  }
  
  Configuration * _this = const_cast<Configuration*>(this);
  
  value = parameters.find(name)->second.value;
  _this->parameters.find(name)->second.checked = true;
  GRAMODS_DEBUG_LOG_I("Read " << name << " = " << value);
  return true;
}
