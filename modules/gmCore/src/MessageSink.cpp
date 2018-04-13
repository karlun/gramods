
#include <gmCore/MessageSink.hh>

#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMCORE;

void MessageSink::initialize() {
  Console::setDefaultSink(std::static_pointer_cast<MessageSink>(this->shared_from_this()));
}

END_NAMESPACE_GMCORE;
