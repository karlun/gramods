
#include <gmCore/OFactory.hh>
#include <gmCore/ImportLibrary.hh>
#include <gmCore/Configuration.hh>

#ifdef gramods_ENABLE_TinyXML2

#include <gmCore/InvalidArgument.hh>

#include <memory>
#include <string>
#include <vector>

using namespace gramods;

struct Base : gmCore::Object {
  int a;
  void setA(int v) { a = v; }
  GM_OFI_DECLARE;
};

GM_OFI_DEFINE(Base);
GM_OFI_PARAM(Base, a, int, Base::setA);

struct Sub : Base {
  int b;
  std::shared_ptr<Sub> ptr;
  std::vector<std::shared_ptr<Sub>> ptrs;
  void setB(int v) { b = v; }
  void setPtr(std::shared_ptr<Sub> p) { ptr = p; }
  void addPtr(std::shared_ptr<Sub> p) { ptrs.push_back(p); }
  std::string getDefaultKey() override { return "blurb"; }
  GM_OFI_DECLARE;
};

GM_OFI_DEFINE_SUB(Sub, Base);
GM_OFI_PARAM(Sub, b, int, Sub::setB);
GM_OFI_POINTER(Sub, ptr, Sub, Sub::setPtr);
GM_OFI_POINTER(Sub, ptrs, Sub, Sub::addPtr);

std::string xml = R"lang=xml(
<config>
  <Base a="412"/>
  <Sub a="517" b="917">
    <Sub KEY="ptr" a="1" b="4"/>
  </Sub>
</config>
)lang=xml";

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

std::string xml2 = R"lang=xml(
<config>
  <Base a="412"/>
  <Sub a="517" b="917">
    <Sub KEY="ptrs" a="1" b="4"/>
    <Sub KEY="ptrs" a="2" b="8"/>
    <Sub KEY="ptrs" a="3" b="12"/>
  </Sub>
</config>
)lang=xml";

TEST(gmCoreBaseFunctionality, Multiple) {
  
  gmCore::Configuration config(xml2);

  std::shared_ptr<Sub> sub;
  EXPECT_TRUE(config.getObject(sub));
  EXPECT_TRUE(sub);
  if (!sub) return;
  if (sub->ptr) return;
  EXPECT_EQ(3, sub->ptrs.size());
  if (sub->ptrs.size() != 3) return;
  EXPECT_EQ(1, sub->ptrs[0]->a);
  EXPECT_EQ(8, sub->ptrs[1]->b);
  EXPECT_EQ(3, sub->ptrs[2]->a);
}

TEST(gmCoreBaseFunctionality, Named) {

  std::string xml = R"lang=xml(
  <config>
    <Base DEF="abc" a="123"/>
    <Base DEF="xyz" a="321"/>
    <Base DEF="UVW" a="999"/>
    <Base DEF="UVw" KEY="bbt" a="999"/>
    <Base KEY="qqq" a="999"/>
    <Base a="999"/>
  </config>
  )lang=xml";

  gmCore::Configuration config(xml);
  std::shared_ptr<Base> base_abc;
  std::shared_ptr<Base> base_xyz;
  std::shared_ptr<Base> base_uvw;
  std::shared_ptr<Base> base_bbt;
  std::shared_ptr<Base> base_qqq;
  std::shared_ptr<Base> base_Base;

  EXPECT_TRUE(config.getObjectByDef("abc", base_abc));
  EXPECT_FALSE(config.getObjectByDef("xYz", base_xyz));
  EXPECT_FALSE(config.getObjectByDef("xyzz", base_xyz));
  EXPECT_FALSE(config.getObjectByDef(" xyz", base_xyz));
  EXPECT_TRUE(config.getObjectByDef("xyz", base_xyz));
  EXPECT_FALSE(config.getObjectByDef("uvw", base_uvw));
  EXPECT_TRUE(config.getObjectByDef("UVW", base_uvw));
  EXPECT_TRUE(config.getObjectByDef("UVW", base_bbt));
  EXPECT_TRUE(config.getObjectByKey("qqq", base_qqq));
  EXPECT_TRUE(config.getObjectByKey("object", base_Base));

  if (base_abc) EXPECT_EQ(123, base_abc->a);
  if (base_xyz) EXPECT_EQ(321, base_xyz->a);
  if (base_uvw) EXPECT_EQ(999, base_uvw->a);
}

TEST(gmCoreBaseFunctionality, ConfigCommandLine) {

  char arg0[] = "test";
  char arg1[] = "--asdf";
  char arg2[] = "--xml";
  char arg3[] = R"lang=xml(
  <config>
    <Base a="412"/>
    <Sub a="517" b="917">
      <Sub KEY="ptrs" a="1" b="4"/>
      <Sub KEY="ptrs" a="2" b="8"/>
      <Sub KEY="ptrs" a="3" b="12"/>
    </Sub>
  </config>
  )lang=xml";

  {
    char *argv[] = { arg0, arg1 };
    int argc = 2;
    EXPECT_THROW(gmCore::Configuration config(argc, argv),
                 gmCore::InvalidArgument);

    std::vector<std::string> errors;
    EXPECT_THROW(gmCore::Configuration config(argc, argv, &errors),
                 gmCore::InvalidArgument);
    EXPECT_TRUE(errors.empty());
  }

  {
    char *argv[] = { arg0, arg1, arg2 };
    int argc = 3;
    EXPECT_THROW(gmCore::Configuration config(argc, argv),
                 gmCore::InvalidArgument);

    std::vector<std::string> errors;
    EXPECT_THROW(gmCore::Configuration config(argc, argv, &errors),
                 gmCore::InvalidArgument);
    EXPECT_TRUE(errors.empty());
  }

  {
    char *argv[] = { arg0, arg1, arg2, arg3, arg1 };
    int argc = 5;
    EXPECT_NO_THROW(gmCore::Configuration config(argc, argv));
    EXPECT_EQ(argc, 3);
    EXPECT_EQ(std::string(arg1), std::string(argv[2]));
  }

  {
    char *argv[] = { arg0, arg1, arg2, arg3, arg1, arg2, arg3 };
    int argc = 7;
    try {
      gmCore::Configuration config(argc, argv);
      EXPECT_EQ(argc, 3);
      EXPECT_EQ(std::string(arg1), std::string(argv[2]));

      std::vector
        <std::shared_ptr<Sub>> subs;
      EXPECT_EQ(2, config.getAllObjects(subs));
    }
    catch(...) {}
  }

  {
    char param[] = "--param";
    char valueA[] = "object.a=199";
    char valueB[] = "Sub.ptrs.a=299";
    char *argv[] = { arg2, arg3, param, valueA, param, valueB };
    int argc = 6;

    gmCore::Configuration config(argc, argv);
    EXPECT_EQ(argc, 0);

    std::shared_ptr<Base> base;
    EXPECT_TRUE(config.getObject(base));
    EXPECT_TRUE(base);
    EXPECT_EQ(199, base->a);

    std::shared_ptr<Sub> sub;
    EXPECT_TRUE(config.getObject(sub));
    EXPECT_TRUE(sub);
    if (!sub) return;
    if (sub->ptr) return;
    EXPECT_EQ(3, sub->ptrs.size());
    if (sub->ptrs.size() != 3) return;
    EXPECT_EQ(299, sub->ptrs[0]->a);
    EXPECT_EQ(8, sub->ptrs[1]->b);
    EXPECT_EQ(299, sub->ptrs[2]->a);

  }
}


struct Multi : gmCore::Object {
  std::vector<int> a;
  void addA(int v) { a.push_back(v); }
  GM_OFI_DECLARE;
};

GM_OFI_DEFINE(Multi);
GM_OFI_PARAM(Multi, a, int, Multi::addA);

TEST(gmCoreBaseFunctionality, MultipleParameters) {

  char arg0[] = "--xml";
  char arg1[] = R"lang=xml(
  <config>
    <Multi a="312"/>
    <Multi a="174">
      <param name="a" value="1391"/>
      <param name="a" value="31"/>
      <param name="a" value="74"/>
    </Multi>
  </config>
  )lang=xml";

  char *argv[] = { arg0, arg1 };
  int argc = 2;

  gmCore::Configuration config(argc, argv);
  EXPECT_EQ(argc, 0);

  std::vector<std::shared_ptr<Multi>> multis;
  EXPECT_TRUE(config.getAllObjects(multis));
  ASSERT_EQ(multis.size(), 2);
  EXPECT_EQ(multis[0]->a.size(), 1);
  ASSERT_EQ(multis[1]->a.size(), 4);
  EXPECT_EQ(multis[1]->a[0], 174);
  EXPECT_EQ(multis[1]->a[1], 1391);
  EXPECT_EQ(multis[1]->a[2], 31);
  EXPECT_EQ(multis[1]->a[3], 74);

}


struct Types : gmCore::Object {
  std::vector<std::string> strings;
  std::vector<bool> bools;
  void addString(std::string v) { strings.push_back(v); }
  void addBool(bool v) { bools.push_back(v); }
  GM_OFI_DECLARE;
};

GM_OFI_DEFINE(Types);
GM_OFI_PARAM(Types, string, std::string, Types::addString);
GM_OFI_PARAM(Types, bool, bool, Types::addBool);

TEST(gmCoreBaseFunctionality, ParameterTypes) {

  char arg0[] = "--xml";
  char arg1[] = R"lang=xml(
  <config>
    <Types>
      <param name="string" value="1391"/>
      <param name="string" value="12 12 12"/>
      <param name="bool" value="true"/>
      <param name="bool" value="True"/>
      <param name="bool" value="TRUE"/>
      <param name="bool" value="1"/>
      <param name="bool" value="false"/>
      <param name="bool" value="False"/>
      <param name="bool" value="FALSE"/>
      <param name="bool" value="0"/>
    </Types>
  </config>
  )lang=xml";

  char *argv[] = { arg0, arg1 };
  int argc = 2;

  gmCore::Configuration config(argc, argv);

  std::shared_ptr<Types> types;
  EXPECT_TRUE(config.getObject(types));

  ASSERT_EQ(types->strings.size(), 2);
  ASSERT_EQ(types->bools.size(), 8);
  EXPECT_EQ(types->strings[0], "1391");
  EXPECT_EQ(types->strings[1], "12 12 12");
  EXPECT_EQ(types->bools[0], true);
  EXPECT_EQ(types->bools[1], true);
  EXPECT_EQ(types->bools[2], true);
  EXPECT_EQ(types->bools[3], true);
  EXPECT_EQ(types->bools[4], false);
  EXPECT_EQ(types->bools[5], false);
  EXPECT_EQ(types->bools[6], false);
  EXPECT_EQ(types->bools[7], false);

  char arg2[] = R"lang=xml(
  <config>
    <Types>
      <param name="bool" value="2"/>
    </Types>
  </config>
  )lang=xml";
  char *argv2[] = { arg0, arg2 };
  argc = 2;
  EXPECT_THROW(gmCore::Configuration config(argc, argv2), gmCore::InvalidArgument);

  char arg3[] = R"lang=xml(
  <config>
    <Types>
      <param name="bool" value="tRue"/>
    </Types>
  </config>
  )lang=xml";
  char *argv3[] = { arg0, arg3 };
  argc = 2;
  EXPECT_THROW(gmCore::Configuration config(argc, argv3), gmCore::InvalidArgument);

}

#endif
