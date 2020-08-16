
#ifndef GRAMODS_CORE_OSTREAMMESSAGESINK
#define GRAMODS_CORE_OSTREAMMESSAGESINK

#include <gmCore/MessageSink.hh>

#include <mutex>

BEGIN_NAMESPACE_GMCORE;

/**
*/
class OStreamMessageSink
  : public MessageSink {

public:

  OStreamMessageSink();

  /**
     Set the stream to use as output. Use this if you do not want
     smart memory management, for example if your output stream is
     std::cout.
  */
  void setStream(std::ostream *s) {
    std::lock_guard<std::mutex> guard(lock);
    raw_out = s;
    shared_out = nullptr;
  }

  /**
     Set the stream to use as output. Use this if you do want smart
     memory management, for example if the output stream is shared
     between objects and is not memory managed by the operating
     system.
  */
  void setStream(std::shared_ptr<std::ostream> s) {
    std::lock_guard<std::mutex> guard(lock);
    raw_out = nullptr;
    shared_out = s;
  }

  /**
     Set the stream to use as output, as string. Valid values are
     "out" and "err".

     \b XML-attribute: \c stream
  */
  void setStream(std::string name);

  /**
     Activate or deactivate the use of ANSI color escape characters to
     emphasize error and warning messages.

     \b XML-attribute: \c useAnsiColor
  */
  void setUseAnsiColor(bool on) {
    use_ansi_color = on;
  }

  void output(Message msg);

  /**
     Set the level of messages to output. This is an integer 0-4 where
     0 results in only error messages begin printed and 4 results in
     all messages.

     \b XML-attribute: \c level
  */
  void setLevel(int l) { level = l; }

  GM_OFI_DECLARE;

private:

  std::ostream *raw_out;
  std::shared_ptr<std::ostream> shared_out;
  std::mutex lock;

  bool use_ansi_color;
  int level;
};

END_NAMESPACE_GMCORE;

#endif
