
#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>

BEGIN_NAMESPACE_GMCORE;

std::vector<std::shared_ptr<MessageSink>> Console::message_sinks;
std::mutex Console::lock;

int Console::ConsoleBuffer::sync() {

  MessageSink::Message m = message_template;
  m.message = str();

  if (sinks.empty())
    getDefaultSink()->output(m);
  else
    for (auto sink : sinks)
      sink->output(m);

  str("");

  return 0;
}

Console::ConsoleBuffer Console::getBuffer
(ConsoleLevel level, std::string tag) {
  std::lock_guard<std::mutex> guard(lock);
  return ConsoleBuffer(message_sinks,
                       MessageSink::Message(level, tag, ""));
}

Console::ConsoleBuffer Console::getBuffer
(ConsoleLevel level, std::string tag,
 std::string file, int line, std::string function) {
  std::lock_guard<std::mutex> guard(lock);
  return ConsoleBuffer(message_sinks,
                       MessageSink::Message(level, tag, "", file, line, function));
}

MessageSink *Console::getDefaultSink() {
  static std::shared_ptr<OStreamMessageSink> sink =
      std::make_shared<OStreamMessageSink>();
  return sink.get();
}

END_NAMESPACE_GMCORE;
