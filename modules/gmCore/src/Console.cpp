
#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>

#include <stack>

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

namespace {
std::stack<std::vector<std::shared_ptr<MessageSink>>> message_sink_stack;
}

void Console::pushSinks() {
  GM_DBG1("Console", "Output suspended because of stack push");
  std::lock_guard<std::mutex> guard(lock);
  message_sink_stack.push({});
  message_sink_stack.top().swap(message_sinks);
}

void Console::popSinks() {
  {
    std::lock_guard<std::mutex> guard(lock);
    message_sink_stack.top().swap(message_sinks);
    message_sink_stack.pop();
  }
  GM_DBG1("Console", "Output back online because of stack pop");
}

END_NAMESPACE_GMCORE;
