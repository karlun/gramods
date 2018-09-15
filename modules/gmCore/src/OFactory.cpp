
#include <gmCore/OFactory.hh>

#include <assert.h>

BEGIN_NAMESPACE_GMCORE;

Object * OFactory::createObject(std::string name){
  if( getOFI(name) == NULL ){
    return NULL; }
  return getOFI(name)->create();
}

std::map<std::string,OFactory::OFactoryInformation*>& OFactory::getOFIByNameMap(){
  static std::map<std::string,OFactoryInformation*> ofi_by_name;
  return ofi_by_name;
}

void OFactory::registerOFI(std::string name, OFactoryInformation *info){
  assert( getOFIByNameMap().count(name) == 0 );

  getOFIByNameMap()[name] = info;
}

void OFactory::unregisterOFI(std::string name){
  assert( getOFIByNameMap().count(name) == 1 );

  if( getOFIByNameMap().count(name) == 1 ){
    getOFIByNameMap().erase(name); }
}

OFactory::OFactoryInformation* OFactory::getOFI(std::string name){
  if( getOFIByNameMap().count(name) == 0 ){
    return NULL; }
  return getOFIByNameMap()[name];
}

OFactory::OFactoryInformation::OFactoryInformation
(std::string name,
 ObjectCreatorBase *creator,
 OFactoryInformation *base)
  : name(name),
    creator(creator),
    base(base) {
  OFactory::registerOFI(name, this);
}

/** Unregisters the class at the object factory. */
OFactory::OFactoryInformation::~OFactoryInformation(){
  OFactory::unregisterOFI(name);
}

Object * OFactory::OFactoryInformation::create() {
  creator->create();
}

void OFactory::OFactoryInformation::registerParamSetter
(std::string name, ParamSetterBase *setter) {
  assert(param_setters.count(name) == 0);
  param_setters[name].reset(setter);
}

void OFactory::OFactoryInformation::registerPointerSetter
(std::string name, PointerSetterBase *setter) {
  assert(pointer_setters.count(name) == 0);
  pointer_setters[name].reset(setter);
}

bool OFactory::OFactoryInformation::setParamValueFromString
(Object *node, std::string name, std::string value) {
  if (param_setters.count(name) == 0)
    if (base == nullptr)
      return false;
    else
      return base->setParamValueFromString(node, name, value);
  param_setters[name]->setValueFromString(node, value);
  return true;
}

bool OFactory::OFactoryInformation::setPointerValue
(Object *node, std::string name, std::shared_ptr<Object> ptr) {
  if (pointer_setters.count(name) == 0)
    if (base == nullptr)
      return false;
    else
      return base->setPointerValue(node, name, ptr);
  pointer_setters[name]->setPointer(node, ptr);
  return true;
}


END_NAMESPACE_GMCORE;
