
#ifndef GRAMODS_CORE_LOGFILEMESSAGESINK
#define GRAMODS_CORE_LOGFILEMESSAGESINK

#include <gmCore/path.hh>
#include <gmCore/MessageSink.hh>

#include <mutex>
#include <fstream>

BEGIN_NAMESPACE_GMCORE;

/**
   Message sink that writes to a specified log file.
*/
class LogFileMessageSink
  : public MessageSink {

public:

  LogFileMessageSink();

  /**
     Sets the path to the log file to write to.

     \gmXmlTag{gmCore,LogFileMessageSink,logFilePath}
  */
  void setLogFilePath(std::filesystem::path path);

  /**
     Set to true if the messages should be appended to the end of the
     file, if it already exists. With false (default) the file will be
     overwritten.

     \gmXmlTag{gmCore,LogFileMessageSink,append}
  */
  void setAppend(bool on);

  void output(Message msg);

  GM_OFI_DECLARE;

private:

  bool append;
  std::filesystem::path logfile_path;
  std::ofstream logfile;

  std::mutex lock;
};

END_NAMESPACE_GMCORE;

#endif
