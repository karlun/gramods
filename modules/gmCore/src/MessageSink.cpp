
#include <gmCore/MessageSink.hh>

#include <gmCore/Console.hh>

#include <iomanip>

BEGIN_NAMESPACE_GMCORE;

GM_OFI_DEFINE_ABSTRACT(MessageSink);
GM_OFI_PARAM2(MessageSink, showTime, bool, setShowTime);

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
  case ConsoleLevel::Debug1:
    out << "D1"; break;
  case ConsoleLevel::Debug2:
    out << "D2"; break;
  case ConsoleLevel::Debug3:
    out << "D3"; break;
  default:
    assert(0);
  }

  if (show_time) {
    typedef std::chrono::duration<double, std::ratio<1>> d_seconds;
    auto since_epoch = std::chrono::duration_cast<d_seconds>(msg.getDuration());
    (std::ostream(out.rdbuf()) << std::setprecision(3) << std::fixed)
        << " " << since_epoch.count() << " ";
  }

  if (msg.tag.length() == 0) out << ": "; 
  else out << " (" << msg.tag << ") ";
}

END_NAMESPACE_GMCORE;
