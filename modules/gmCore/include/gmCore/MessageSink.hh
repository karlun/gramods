
#ifndef GRAMODS_CORE_MESSAGESINK
#define GRAMODS_CORE_MESSAGESINK

#include <gmCore/config.hh>

#include <gmCore/ConsoleLevel.hh>
#include <gmCore/Object.hh>
#include <ostream>

BEGIN_NAMESPACE_GMCORE;

/**
   This is the base type for back-ends taking care of information sent
   to the Console class. A sink will register itself as the default
   sink of the Console class upon initialization.
*/
class MessageSink
  : public gmCore::Object {

public:

  struct Message {

    Message(ConsoleLevel level, std::string tag, std::string msg,
            std::string file, int line, std::string function)
      : level(level), tag(tag), message(msg),
        source_data_available(true), file(file), line(line), function(function) {}

    Message(ConsoleLevel level, std::string tag, std::string msg)
      : level(level), tag(tag), message(msg),
        source_data_available(false), file(""), line(0), function("") {}

    ConsoleLevel level;
    std::string tag;
    std::string message;
    bool source_data_available;
    std::string file;
    int line;
    std::string function;
  };

  /**
     Called to initialize the MessageSink. This should be called once
     only!
  */
  virtual void initialize();

  /**
     Outputs the provided message to the implementation specific
     channel. This method may be called concurrently from multiple
     thread and must therefore be thread safe.
  */
  virtual void output(Message msg) = 0;

};

END_NAMESPACE_GMCORE;

#endif
