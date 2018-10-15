
#include <gmCore/Updateable.hh>

#include <set>

BEGIN_NAMESPACE_GMCORE;

struct Updateable::Impl {

  Impl(Updateable * _this);
  ~Impl();

  Updateable * _this;

  static void updateAll(clock::time_point t);

  static std::set<Updateable*>& getList();

};

Updateable::Updateable()
  : _impl(new Impl(this)) {}

Updateable::~Updateable() {
  delete _impl;
  _impl = nullptr;
}

Updateable::Impl::Impl(Updateable * _this)
  : _this(_this) {
  getList().insert(_this);
}

Updateable::Impl::~Impl() {
  getList().erase(_this);
}

void Updateable::updateAll(clock::time_point t) {
  Impl::updateAll(t);
}

std::set<Updateable*>& Updateable::Impl::getList() {
  static std::set<Updateable*> list;
  return list;
}

void Updateable::Impl::updateAll(clock::time_point t) {
  for (auto it : getList())
    it->update(t);
}

END_NAMESPACE_GMCORE;
