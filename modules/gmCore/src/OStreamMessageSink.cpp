
#include <gmCore/OStreamMessageSink.hh>

#include <gmCore/OFactory.hh>

#include <iostream>
#include <assert.h>

BEGIN_NAMESPACE_GMCORE;

OFI_CREATE(OStreamMessageSinkOFI, OStreamMessageSink);
OFI_PARAM(OStreamMessageSinkOFI, OStreamMessageSink, stream, std::string, OStreamMessageSink::setStream);

OStreamMessageSink::OStreamMessageSink()
  : rp_out(&std::cerr) {}

void OStreamMessageSink::output(Message msg) {

  if (rp_out == nullptr && !sp_out) return;

  std::ostream &out = rp_out != nullptr ? *rp_out : *sp_out.get();

  if (msg.source_data_available) {
    outputLevelAndTag(msg);
    out << msg.file << ":" << msg.line << " (" << msg.function << ")" << std::endl;
  }

  outputLevelAndTag(msg);

  out << msg.message << std::endl;
}

void OStreamMessageSink::outputLevelAndTag(Message msg) {
  std::ostream &out = rp_out != nullptr ? *rp_out : *sp_out.get();

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
  if (name == "out") {
    setStream(&std::cout);
  } else if (name == "err") {
    setStream(&std::cerr);
  } else {
    setStream(nullptr);
  }
}

END_NAMESPACE_GMCORE;
