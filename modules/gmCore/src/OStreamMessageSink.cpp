
#include <gmCore/OStreamMessageSink.hh>

#include <iostream>
#include <assert.h>

BEGIN_NAMESPACE_GMCORE;

GM_OFI_DEFINE(OStreamMessageSink);
GM_OFI_PARAM(OStreamMessageSink, stream, std::string, OStreamMessageSink::setStream);
GM_OFI_PARAM(OStreamMessageSink, useAnsiColor, bool, OStreamMessageSink::setUseAnsiColor);

OStreamMessageSink::OStreamMessageSink()
  : raw_out(&std::cerr),
    use_ansi_color(false) {}

void OStreamMessageSink::output(Message msg) {
  std::lock_guard<std::mutex> guard(lock);

  if (raw_out == nullptr && !shared_out) return;

  std::ostream &out = raw_out != nullptr ? *raw_out : *shared_out.get();

  if (msg.source_data_available) {
    outputLevelAndTag(msg, out);
    out << msg.file << ":" << msg.line << " (" << msg.function << ")" << std::endl;
  }

  outputLevelAndTag(msg, out);

  out << msg.message;

  if (use_ansi_color) out << "\033[37m";
}

void OStreamMessageSink::outputLevelAndTag(Message msg, std::ostream &out) {
  switch (msg.level) {
  case ConsoleLevel::ERROR:
    if (use_ansi_color) out << "\033[31m";
    out << "EE";
    break;
  case ConsoleLevel::WARNING:
    if (use_ansi_color) out << "\033[33m";
    out << "WW";
    break;
  case ConsoleLevel::INFORMATION:
    if (use_ansi_color) out << "\033[37m";
    out << "II";
    break;
  case ConsoleLevel::VERBOSE_INFORMATION:
    if (use_ansi_color) out << "\033[37m";
    out << "I2";
    break;
  case ConsoleLevel::VERY_VERBOSE_INFORMATION:
    if (use_ansi_color) out << "\033[37m";
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
