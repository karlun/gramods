/**
   Copyright 2003-2018, Karljohan Lundin Palmerius
   Copyright 2010, Loki Astari (http://stackoverflow.com)
   
*/


#ifndef GRAMODS_IO_CONSOLE
#define GRAMODS_IO_CONSOLE

#include <gmIO/config.hh>

#include <gmIO/ConsoleLevel.hh>
#include <gmIO/MessageSink.hh>
#include <memory>
#include <sstream>

#ifdef NDEBUG

# define GM_ERR(TAG, MSG)                                              \
  gramods::gmIO::Console(gramods::gmIO::ConsoleLevel::ERROR,           \
                         TAG) << MSG << std::endl
# define GM_WRN(TAG, MSG)                                              \
  gramods::gmIO::Console(gramods::gmIO::ConsoleLevel::WARNING,        \
                         TAG) << MSG << std::endl
# define GM_INF(TAG, MSG)                                            \
  gramods::gmIO::Console(gramods::gmIO::ConsoleLevel::INFORMATION,  \
                         TAG) << MSG << std::endl
# define GM_VINF(TAG, MSG)                                              \
  gramods::gmIO::Console(gramods::gmIO::ConsoleLevel::VERBOSE_INFORMATION, \
                         TAG) << MSG << std::endl
# define GM_VVINF(TAG, MSG)                                             \
  gramods::gmIO::Console(gramods::gmIO::ConsoleLevel::VERY_VERBOSE_INFORMATION, \
                         TAG) << MSG << std::endl

#else // if NDEBUG else

# define GM_ERR(TAG, MSG)                                              \
  gramods::gmIO::Console(gramods::gmIO::ConsoleLevel::ERROR,           \
                         TAG, __FILE__, __LINE__, __func__)            \
  << MSG << std::endl
# define GM_WRN(TAG, MSG)                                             \
  gramods::gmIO::Console(gramods::gmIO::ConsoleLevel::WARNING,        \
                         TAG, __FILE__, __LINE__, __func__)           \
  << MSG << std::endl
# define GM_INF(TAG, MSG)                                            \
  gramods::gmIO::Console(gramods::gmIO::ConsoleLevel::INFORMATION,   \
                         TAG, __FILE__, __LINE__, __func__)          \
  << MSG << std::endl
# define GM_VINF(TAG, MSG)                                              \
  gramods::gmIO::Console(gramods::gmIO::ConsoleLevel::VERBOSE_INFORMATION, \
                         TAG, __FILE__, __LINE__, __func__)             \
  << MSG << std::endl
# define GM_VVINF(TAG, MSG)                                             \
  gramods::gmIO::Console(gramods::gmIO::ConsoleLevel::VERY_VERBOSE_INFORMATION, \
                         TAG, __FILE__, __LINE__, __func__)             \
  << MSG << std::endl

#endif // if NDEBUG else endif

BEGIN_NAMESPACE_GMIO

/**
   Console for easier handling of runtime and debugging information.
*/
struct Console
  : public std::ostream {

public:

  Console(ConsoleLevel level, std::string tag,
          std::string file, int line, std::string function)
    : std::ostream(&buffer),
    buffer(getBuffer(level, tag, file, line, function)) {}

  Console(ConsoleLevel level, std::string tag)
    : std::ostream(&buffer),
    buffer(getBuffer(level, tag)) {}

  static void setDefaultSink(std::shared_ptr<MessageSink> ms) {
    default_message_sink = ms;
  }

private:

  class ConsoleBuffer
      : public std::stringbuf {
    std::shared_ptr<MessageSink> sink;
    MessageSink::Message message_template;
  public:
    ConsoleBuffer(std::shared_ptr<MessageSink> sink,
                  MessageSink::Message msg)
      : sink(sink),
        message_template(msg) {}
    virtual int sync();
  };

  static ConsoleBuffer getBuffer(ConsoleLevel level, std::string tag);

  static ConsoleBuffer getBuffer(ConsoleLevel level, std::string tag,
                                 std::string file, int line, std::string function);

  static std::shared_ptr<MessageSink> default_message_sink;

  ConsoleBuffer buffer;
};

END_NAMESPACE_GMIO

#endif
