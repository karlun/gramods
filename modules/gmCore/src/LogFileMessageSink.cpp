
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
  logfile.flush();
}

void LogFileMessageSink::outputLevelAndTag(std::ostream &out, Message msg) {
  switch (msg.level) {
  case ConsoleLevel::error:
    out << "EE"; break;
  case ConsoleLevel::warning:
    out << "WW"; break;
  case ConsoleLevel::information:
    out << "II"; break;
  case ConsoleLevel::verbose_information:
    out << "I2"; break;
  case ConsoleLevel::very_verbose_information:
    out << "I3"; break;
  default:
    assert(0);
  }

  if (msg.tag.length() == 0) out << ": "; 
  else out << " (" << msg.tag << ") ";
}


END_NAMESPACE_GMCORE;
