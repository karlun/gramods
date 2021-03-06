
#include <gmCore/LogFileMessageSink.hh>

#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMCORE;

GM_OFI_DEFINE_SUB(LogFileMessageSink, MessageSink);
GM_OFI_PARAM2(LogFileMessageSink, logFilePath, std::filesystem::path, setLogFilePath);
GM_OFI_PARAM2(LogFileMessageSink, append, bool, setAppend);

LogFileMessageSink::LogFileMessageSink()
  : append(false) {}

void LogFileMessageSink::setLogFilePath(std::filesystem::path path) {
  std::lock_guard<std::mutex> guard(lock);
  logfile_path = path;
  logfile.close();
}

void LogFileMessageSink::setAppend(bool on) {
  std::lock_guard<std::mutex> guard(lock);
  append = on;
}

void LogFileMessageSink::output(Message msg) {
  std::lock_guard<std::mutex> guard(lock);
  if (logfile_path.empty())
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

  std::istringstream input(msg.message);
  for (std::string line; std::getline(input, line);) {
    outputMetadata(logfile, msg);
    logfile << line << "\n";
  }
  logfile.flush();
}

END_NAMESPACE_GMCORE;
