
#include <gmCore/LogFileMessageSink.hh>

#include <gmCore/Console.hh>
#include <gmCore/FileResolver.hh>

BEGIN_NAMESPACE_GMCORE;

GM_OFI_DEFINE_SUB(LogFileMessageSink, MessageSink);
GM_OFI_PARAM2(LogFileMessageSink, logFilePath, std::filesystem::path, setLogFilePath);
GM_OFI_PARAM2(LogFileMessageSink, append, bool, setAppend);
GM_OFI_PARAM2(LogFileMessageSink, level, int, setLevel);

LogFileMessageSink::LogFileMessageSink()
  : append(false) {}

void LogFileMessageSink::setLogFilePath(std::filesystem::path path) {
  std::lock_guard<std::mutex> guard(lock);
  logfile_path = gmCore::FileResolver::getDefault()->resolve(
      path, gmCore::FileResolver::Check::WritableFile);
  logfile.close();
}

void LogFileMessageSink::setAppend(bool on) {
  std::lock_guard<std::mutex> guard(lock);
  append = on;
}

void LogFileMessageSink::output(Message msg) {
  if (msg.level > gramods::gmCore::ConsoleLevel(level)) return;
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
