
#include <gmCore/OFactory.hh>
#include <gmCore/ImportLibrary.hh>
#include <gmCore/Configuration.hh>

#include <memory>
#include <string>
#include <vector>

using namespace gramods;

struct Base : gmCore::Object {
  int a;
  void setA(int v) { a = v; }
};

OFI_CREATE(BaseOFI, Base);
OFI_PARAM(BaseOFI, Base, a, int, Base::setA);

struct Sub : Base {
  int b;
  std::shared_ptr<Sub> ptr;
  std::vector<std::shared_ptr<Sub>> ptrs;
  void setB(int v) { b = v; }
  void setPtr(std::shared_ptr<Sub> p) { ptr = p; }
  void addPtr(std::shared_ptr<Sub> p) { ptrs.push_back(p); }
};

OFI_CREATE_SUB(SubOFI, Sub, &BaseOFI);
OFI_PARAM(SubOFI, Sub, b, int, Sub::setB);
OFI_POINTER(SubOFI, Sub, ptr, Sub, Sub::setPtr);
OFI_POINTER(SubOFI, Sub, ptrs, Sub, Sub::addPtr);

std::string xml = ""
  "<config>"
  "  <Base a=\"412\"/>"
  "  <Sub a=\"517\" b=\"917\">"
  "    <Sub AS=\"ptr\" a=\"1\" b=\"4\"/>"
  "  </Sub>"
  "</config>";

TEST(gmCoreBaseFunctionality, Inheritance) {
  
  gmCore::Configuration config(xml);

  std::shared_ptr<Base> base;
  EXPECT_TRUE(config.getObject(base));
  EXPECT_TRUE(base);
  EXPECT_EQ(412, base->a);

  std::shared_ptr<Sub> sub;
  EXPECT_TRUE(config.getObject(sub));
  EXPECT_TRUE(sub);
  if (sub) {
    EXPECT_EQ(517, sub->a);
    EXPECT_EQ(917, sub->b);
    if (sub->ptr) {
      EXPECT_TRUE(sub->ptr);
      EXPECT_EQ(1, sub->ptr->a);
      EXPECT_EQ(4, sub->ptr->b);
    }
  }
}

TEST(gmCoreBaseFunctionality, Tree) {
  
  gmCore::Configuration config(xml);

  std::shared_ptr<Sub> sub;
  EXPECT_TRUE(config.getObject(sub));
  EXPECT_TRUE(sub);
  if (sub && sub->ptr) {
    EXPECT_TRUE(sub->ptr);
    EXPECT_EQ(1, sub->ptr->a);
    EXPECT_EQ(4, sub->ptr->b);
  }
}

std::string xml2 = ""
  "<config>"
  "  <Base a=\"412\"/>"
  "  <Sub a=\"517\" b=\"917\">"
  "    <Sub AS=\"ptrs\" a=\"1\" b=\"4\"/>"
  "    <Sub AS=\"ptrs\" a=\"2\" b=\"8\"/>"
  "    <Sub AS=\"ptrs\" a=\"3\" b=\"12\"/>"
  "  </Sub>"
  "</config>";

TEST(gmCoreBaseFunctionality, Multiple) {
  
  gmCore::Configuration config(xml2);

  std::shared_ptr<Sub> sub;
  EXPECT_TRUE(config.getObject(sub));
  EXPECT_TRUE(sub);
  if (!sub) return;
  if (sub->ptr) return;
  EXPECT_EQ(3, sub->ptrs.size());
  if (sub->ptrs.size() != 3);
  EXPECT_EQ(1, sub->ptrs[0]->a);
  EXPECT_EQ(8, sub->ptrs[1]->b);
  EXPECT_EQ(3, sub->ptrs[2]->a);
}
