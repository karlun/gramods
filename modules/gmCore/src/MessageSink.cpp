
#include <gmCore/MessageSink.hh>

#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMCORE;

GM_OFI_DEFINE_ABSTRACT(MessageSink);
GM_OFI_PARAM(MessageSink, showTime, bool, MessageSink::setShowTime);

void MessageSink::initialize() {
  Console::addSink(std::static_pointer_cast<MessageSink>(this->shared_from_this()));
  Object::initialize();
}

void MessageSink::outputMetadata(std::ostream &out, Message msg) {

  switch (msg.level) {
  case ConsoleLevel::Error:
    out << "EE"; break;
  case ConsoleLevel::Warning:
    out << "WW"; break;
  case ConsoleLevel::Information:
    out << "II"; break;
  case ConsoleLevel::VerboseInformation:
    out << "I2"; break;
  case ConsoleLevel::VeryVerboseInformation:
    out << "I3"; break;
  default:
    assert(0);
  }

  if (show_time) {
    typedef std::chrono::duration<double, std::ratio<1>> d_seconds;
    auto since_epoch = std::chrono::duration_cast<d_seconds>(msg.getDuration());
    out << " " << 0.001 * (size_t)(1000 * since_epoch.count()) << " ";
  }

  if (msg.tag.length() == 0) out << ": "; 
  else out << " (" << msg.tag << ") ";
}

END_NAMESPACE_GMCORE;
