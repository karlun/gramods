
#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMCORE;

std::vector<std::shared_ptr<MessageSink>> Console::message_sinks;

int Console::ConsoleBuffer::sync() {

  MessageSink::Message m = message_template;
  m.message = str();

  for (auto sink : sinks)
    sink->output(m);

  str("");

  return 0;
}

Console::ConsoleBuffer Console::getBuffer
(ConsoleLevel level, std::string tag) {
  return ConsoleBuffer(message_sinks,
                       MessageSink::Message(level, tag, ""));
}

Console::ConsoleBuffer Console::getBuffer
(ConsoleLevel level, std::string tag,
 std::string file, int line, std::string function) {
  return ConsoleBuffer(message_sinks,
                       MessageSink::Message(level, tag, "", file, line, function));
}

END_NAMESPACE_GMCORE;
