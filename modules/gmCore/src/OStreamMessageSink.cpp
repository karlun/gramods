
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/Stringify.hh>

#include <iostream>
#include <assert.h>

BEGIN_NAMESPACE_GMCORE;

GM_OFI_DEFINE_SUB(OStreamMessageSink, MessageSink);
GM_OFI_PARAM(OStreamMessageSink, stream, std::string, OStreamMessageSink::setStream);
GM_OFI_PARAM(OStreamMessageSink, useAnsiColor, bool, OStreamMessageSink::setUseAnsiColor);
GM_OFI_PARAM(OStreamMessageSink, level, int, OStreamMessageSink::setLevel);

#define ANSI_NORMAL  "\033[37m"
#define ANSI_ERROR   "\033[91m"
#define ANSI_WARNING "\033[93m"


OStreamMessageSink::OStreamMessageSink()
  : raw_out(&std::cerr),
    use_ansi_color(false),
    level(4) {}

void OStreamMessageSink::output(Message msg) {
  if (msg.level > gramods::gmCore::ConsoleLevel(level)) return;
  std::lock_guard<std::mutex> guard(lock);

  if (raw_out == nullptr && !shared_out) return;

  std::ostream &out = raw_out != nullptr ? *raw_out : *shared_out.get();

  if (use_ansi_color)
    switch (msg.level) {
    case ConsoleLevel::Error:
      out << ANSI_ERROR; break;
    case ConsoleLevel::Warning:
      out << ANSI_WARNING; break;
    case ConsoleLevel::Information:
    case ConsoleLevel::VerboseInformation:
    case ConsoleLevel::VeryVerboseInformation:
      out << ANSI_NORMAL; break;
    }

#ifndef NDEBUG
  if (msg.source_data_available) {
    outputMetadata(out, msg);
    out << msg.file << ":" << msg.line << " (" << msg.function << ")" << std::endl;
  }
#endif

  outputMetadata(out, msg);
  out << msg.message;

  if (use_ansi_color) out << ANSI_NORMAL;
}

void OStreamMessageSink::setStream(std::string name) {
  // no mutex lock since setStream engages the mutex
  if (name == "out") {
    setStream(&std::cout);
  } else if (name == "err") {
    setStream(&std::cerr);
  } else {
    throw std::invalid_argument(GM_STR("invalid stream name '" << name << "'"));
  }
}

END_NAMESPACE_GMCORE;
