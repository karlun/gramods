
#include <gmCore/Updateable.hh>

#include <map>
#include <algorithm>

BEGIN_NAMESPACE_GMCORE;

struct Updateable::Impl {

  Impl(Updateable * _this, int priority);
  ~Impl();

  Updateable * _this;

  static void updateAll(clock::time_point t);

  static std::multimap<int, Updateable*>& getList();

};

Updateable::Updateable(int priority)
  : _impl(new Impl(this, priority)) {}

Updateable::~Updateable() {
  delete _impl;
  _impl = nullptr;
}

Updateable::Impl::Impl(Updateable * _this, int priority)
  : _this(_this) {
  // Negate to sort highest priority first
  getList().insert(std::make_pair(-priority, _this));
}

Updateable::Impl::~Impl() {
  for(auto it = getList().begin(); it != getList().end(); )
    if (_this == it->second)
      it = getList().erase(it++);
    else
      ++it;
}

void Updateable::updateAll(clock::time_point t) {
  Impl::updateAll(t);
}

std::multimap<int, Updateable*>& Updateable::Impl::getList() {
  static std::multimap<int, Updateable*> list;
  return list;
}

void Updateable::Impl::updateAll(clock::time_point t) {
  for (auto it : getList())
    it.second->update(t);
}

END_NAMESPACE_GMCORE;
