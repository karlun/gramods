
#include <gmCore/OStreamMessageSink.hh>

#include <iostream>
#include <assert.h>

BEGIN_NAMESPACE_GMCORE;

GM_OFI_DEFINE(OStreamMessageSink);
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

#ifndef NDEBUG
  if (msg.source_data_available) {
    outputLevelAndTag(msg, out);
    out << msg.file << ":" << msg.line << " (" << msg.function << ")" << std::endl;
  }
#endif

  outputLevelAndTag(msg, out);

  out << msg.message;

  if (use_ansi_color) out << ANSI_NORMAL;
}

void OStreamMessageSink::outputLevelAndTag(Message msg, std::ostream &out) {
  switch (msg.level) {
  case ConsoleLevel::Error:
    if (use_ansi_color) out << ANSI_ERROR;
    out << "EE";
    break;
  case ConsoleLevel::Warning:
    if (use_ansi_color) out << ANSI_WARNING;
    out << "WW";
    break;
  case ConsoleLevel::Information:
    if (use_ansi_color) out << ANSI_NORMAL;
    out << "II";
    break;
  case ConsoleLevel::VerboseInformation:
    if (use_ansi_color) out << ANSI_NORMAL;
    out << "I2";
    break;
  case ConsoleLevel::VeryVerboseInformation:
    if (use_ansi_color) out << ANSI_NORMAL;
    out << "I3";
    break;
  default:
    assert(0);
  }

  if (msg.tag.length() == 0) out << ": "; 
  else out << " (" << msg.tag << ") ";
}

void OStreamMessageSink::setStream(std::string name) {
  // no mutex lock since setStream engages the mutex
  if (name == "out") {
    setStream(&std::cout);
  } else if (name == "err") {
    setStream(&std::cerr);
  } else {
    std::stringstream ss;
    ss << "invalid stream name '" << name << "'";
    throw std::invalid_argument(ss.str());
  }
}

END_NAMESPACE_GMCORE;
