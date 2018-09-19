
#include <gmCore/MessageSink.hh>

#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMCORE;

void MessageSink::initialize() {
  Console::addSink(std::static_pointer_cast<MessageSink>(this->shared_from_this()));
  Object::initialize();
}

END_NAMESPACE_GMCORE;
