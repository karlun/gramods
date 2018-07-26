
#include <gmCore/LogFileMessageSink.hh>

BEGIN_NAMESPACE_GMCORE;

GM_OFI_DEFINE(LogFileMessageSink);
GM_OFI_PARAM(LogFileMessageSink, logfilePath, std::string, LogFileMessageSink::setLogFilePath);
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

  logfile << msg.message << std::endl;
}


END_NAMESPACE_GMCORE;
