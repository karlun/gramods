
#ifndef GRAMODS_CORE_MESSAGESINK
#define GRAMODS_CORE_MESSAGESINK

#include <gmCore/config.hh>

#include <gmCore/ConsoleLevel.hh>
#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>

#include <ostream>
#include <chrono>

BEGIN_NAMESPACE_GMCORE;

/**
   This is the base type for back-ends taking care of information sent
   to the Console class. A sink will register itself as a sink of the
   Console class upon initialization. To remove a registered message
   sink, call removeSink or removeAllSinks to remove all sinks.
*/
class MessageSink
  : public gmCore::Object {

public:

  struct Message {

    typedef std::chrono::steady_clock clock;

    Message(ConsoleLevel level, std::string tag, std::string msg,
            std::string file, int line, std::string function)
      : level(level), tag(tag), message(msg),
        source_data_available(true), file(file), line(line), function(function),
        time_stamp(clock::now()) {}

    Message(ConsoleLevel level, std::string tag, std::string msg)
      : level(level), tag(tag), message(msg),
        source_data_available(false), file(""), line(0), function(""),
        time_stamp(clock::now()) {}

    clock::duration getDuration() {
      static clock::time_point start_time = clock::now();
      return time_stamp - start_time;
    }

    ConsoleLevel level;

    std::string tag;
    std::string message;

    bool source_data_available;
    std::string file;
    int line;
    std::string function;

    clock::time_point time_stamp;
  };

  /**
     Called to initialize the MessageSink. This should be called once
     only!
  */
  virtual void initialize() override;

  /**
     Activate or deactivate the output of the time of each
     message. Default is false, not showing time.

     \gmXmlTag{gmCore,MessageSink,showTime}
  */
  void setShowTime(bool on) {
    show_time = on;
  }

  /**
     Outputs the provided message to the implementation specific
     channel. This method may be called concurrently from multiple
     thread and must therefore be thread safe.
  */
  virtual void output(Message msg) = 0;

  GM_OFI_DECLARE;

protected:

  /**
     Write metadata from the specified message to the specified
     stream.
  */
  void outputMetadata(std::ostream &out, Message msg);

private:

  bool show_time = false;
};

END_NAMESPACE_GMCORE;

#endif
