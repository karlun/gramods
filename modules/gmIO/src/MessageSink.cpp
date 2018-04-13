
#include <gmIO/MessageSink.hh>

#include <gmIO/Console.hh>

BEGIN_NAMESPACE_GMIO;

void MessageSink::initialize() {
  Console::setDefaultSink(std::static_pointer_cast<MessageSink>(this->shared_from_this()));
}

END_NAMESPACE_GMIO;
