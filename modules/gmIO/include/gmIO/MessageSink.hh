
#ifndef GRAMODS_IO_MESSAGESINK
#define GRAMODS_IO_MESSAGESINK

#include <gmIO/config.hh>

#include <gmIO/ConsoleLevel.hh>
#include <gmConfig/Object.hh>
#include <ostream>

BEGIN_NAMESPACE_GMIO;

/**
   This is the base type for back-ends taking care of information sent
   to the Console class. A sink will register itself as the default
   sink of the Console class upon initialization.
*/
class MessageSink
  : public gmConfig::Object {

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

  virtual void initialize();

  virtual void output(Message msg) = 0;

};

END_NAMESPACE_GMIO;

#endif
