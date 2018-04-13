
#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMCORE;

std::shared_ptr<MessageSink> Console::default_message_sink;

int Console::ConsoleBuffer::sync() {

  if (!sink) return 0;

  MessageSink::Message m = message_template;
  m.message = str();
  sink->output(m);

  str("");

  return 0;
}

Console::ConsoleBuffer Console::getBuffer
(ConsoleLevel level, std::string tag) {
  return ConsoleBuffer(default_message_sink,
                       MessageSink::Message(level, tag, ""));
}

Console::ConsoleBuffer Console::getBuffer
(ConsoleLevel level, std::string tag,
 std::string file, int line, std::string function) {
  return ConsoleBuffer(default_message_sink,
                       MessageSink::Message(level, tag, "", file, line, function));
}

END_NAMESPACE_GMCORE;
