
#include <gmConfig/OFactory.hh>
#include <gmConfig/ImportLibrary.hh>
#include <gmConfig/Configuration.hh>

#include <memory>
#include <string>

using namespace gramods;

struct Base : gmConfig::Object {
  int a;
  void setA(int v) { a = v; }
};

OFI_CREATE(BaseOFI, Base);
OFI_PARAM(BaseOFI, Base, a, int, Base::setA);

struct Sub : Base {
  int b;
  void setB(int v) { b = v; }
};

OFI_CREATE_SUB(SubOFI, Sub, &BaseOFI);
OFI_PARAM(SubOFI, Sub, b, int, Sub::setB);

std::string xml = ""
  "<config>"
  "  <Base a=\"412\"/>"
  "  <Sub a=\"517\" b=\"917\"/>"
  "</config>";

TEST(gmConfigBaseFunctionality, ) {
  
  gmConfig::Configuration config(xml);

}
