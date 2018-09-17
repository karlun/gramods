
#include <gmCore/OStreamMessageSink.hh>

#include <iostream>
#include <assert.h>

BEGIN_NAMESPACE_GMCORE;

GM_OFI_DEFINE(OStreamMessageSink);
GM_OFI_PARAM(OStreamMessageSink, stream, std::string, OStreamMessageSink::setStream);

OStreamMessageSink::OStreamMessageSink()
  : raw_out(&std::cerr) {}

void OStreamMessageSink::output(Message msg) {
  std::lock_guard<std::mutex> guard(lock);

  if (raw_out == nullptr && !shared_out) return;

  std::ostream &out = raw_out != nullptr ? *raw_out : *shared_out.get();

  if (msg.source_data_available) {
    outputLevelAndTag(msg);
    out << msg.file << ":" << msg.line << " (" << msg.function << ")" << std::endl;
  }

  outputLevelAndTag(msg);

  out << msg.message;
}

void OStreamMessageSink::outputLevelAndTag(Message msg) {
  std::ostream &out = raw_out != nullptr ? *raw_out : *shared_out.get();

  switch (msg.level) {
  case ConsoleLevel::ERROR:
    out << "EE"; break;
  case ConsoleLevel::WARNING:
    out << "WW"; break;
  case ConsoleLevel::INFORMATION:
    out << "II"; break;
  case ConsoleLevel::VERBOSE_INFORMATION:
    out << "I2"; break;
  case ConsoleLevel::VERY_VERBOSE_INFORMATION:
    out << "I3"; break;
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
    setStream(nullptr);
  }
}

END_NAMESPACE_GMCORE;
