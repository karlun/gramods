
#include <gmConfig/OFactoryNode.hh>

#include <gmConfig/Debug.hh>

#include <assert.h>

USING_NAMESPACE_GMCONFIG;

OFactoryNode * OFactoryNode::createObject(std::string name){
  if( getOFI(name) == NULL ){
    return NULL; }
  return getOFI(name)->create();
}

std::map<std::string,OFactoryNode::OFactoryInformationBase*>& OFactoryNode::getOFIByNameMap(){
  static std::map<std::string,OFactoryInformationBase*> ofi_by_name;
  return ofi_by_name;
}

void OFactoryNode::registerOFI(std::string name, OFactoryInformationBase *info){
  assert( getOFIByNameMap().count(name) == 0 );

  getOFIByNameMap()[name] = info;
}

void OFactoryNode::unregisterOFI(std::string name){
  assert( getOFIByNameMap().count(name) == 1 );

  if( getOFIByNameMap().count(name) == 1 ){
    getOFIByNameMap().erase(name); }
}

OFactoryNode::OFactoryInformationBase* OFactoryNode::getOFI(std::string name){
  if( getOFIByNameMap().count(name) == 0 ){
    return NULL; }
  return getOFIByNameMap()[name];
}

