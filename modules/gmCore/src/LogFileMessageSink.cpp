
#include <gmCore/LogFileMessageSink.hh>

BEGIN_NAMESPACE_GMCORE;

GM_OFI_DEFINE(LogFileMessageSink);
GM_OFI_PARAM(LogFileMessageSink, logFilePath, std::string, LogFileMessageSink::setLogFilePath);
GM_OFI_PARAM(LogFileMessageSink, append, bool, LogFileMessageSink::setAppend);

LogFileMessageSink::LogFileMessageSink()
  : append(false) {}

void LogFileMessageSink::setLogFilePath(std::string name) {
  std::lock_guard<std::mutex> guard(lock);
  logfile_path = name;
  logfile.close();
}

void LogFileMessageSink::setAppend(bool on) {
  std::lock_guard<std::mutex> guard(lock);
  append = on;
}

void LogFileMessageSink::output(Message msg) {
  std::lock_guard<std::mutex> guard(lock);
  if (logfile_path == "")
    return;

  if (!logfile) {

    if (append)
      logfile.open(logfile_path, std::ofstream::out | std::ofstream::app);
    else
      logfile.open(logfile_path);

    if (!logfile) {
      GM_ERR("LogFileMessageSink", "Could not open logfile '" << logfile_path << "'");
      logfile_path = "";
      return;
    }
  }

  outputLevelAndTag(logfile, msg);

  logfile << msg.message;
}

void LogFileMessageSink::outputLevelAndTag(std::ostream &out, Message msg) {
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


END_NAMESPACE_GMCORE;
