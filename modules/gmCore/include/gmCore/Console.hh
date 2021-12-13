/**
   Copyright 2003-2018, Karljohan Lundin Palmerius
   Copyright 2010, Loki Astari (http://stackoverflow.com)
   
*/


#ifndef GRAMODS_CORE_CONSOLE
#define GRAMODS_CORE_CONSOLE

#include <gmCore/config.hh>

#include <gmCore/ConsoleLevel.hh>
#include <gmCore/MessageSink.hh>

#include <gmCore/export.hh>

#include <memory>
#include <vector>
#include <sstream>

#include <iostream>
#include <string>
#include <algorithm>
#include <mutex>

BEGIN_NAMESPACE_GMCORE;

/// TODO: replace with std::filesystem::path(__FILE__).filename() when
/// this is supported by mainstream GCC (version > 8.0)

/**
   Local functions for string handling.
*/
namespace detail {
  constexpr bool is_path_sep(char c) {
    return c == '/' || c == '\\';
  }

  constexpr const char* strip_path(const char* path) {
    auto lastname = path;
    for (auto p = path ; *p ; ++p) {
      if (is_path_sep(*p) && *(p+1)) lastname = p+1;
    }
    return lastname;
  }

  struct basename_impl {
    constexpr basename_impl(const char* begin, const char* end)
    : _begin(begin), _end(end) {}

    void write(std::ostream& os) const {
      os.write(_begin, _end - _begin);
    }

    std::string as_string() const {
      return std::string(_begin, _end);
    }

    const char* const _begin;
    const char* const _end;
  };

  inline std::ostream& operator<<(std::ostream& os, const basename_impl& bi) {
    bi.write(os);
    return os;
  }

  inline std::string to_string(const basename_impl& bi) {
    return bi.as_string();
  }

  constexpr const char* last_dot_of(const char* p) {
    const char* last_dot = nullptr;
    for ( ; *p ; ++p) {
      if (*p == '.')
        last_dot = p;
    }
    return last_dot ? last_dot : p;
  }
}

/**\def GM_ERR(TAG, MSG)
   Macro for sending an error message to the currently registered
   MessageSink. The application specific tag may be any string, or an
   empty string. Messages with this level should be very sparse, since
   most error information needs to be provided to the users via the
   user interface instead.

   @param TAG A string to associate the message with, or an empty
   string.

   @param MSG The message to send. This supports the output operator
   for easy printing of objects supporting this operator.
*/

/**\def GM_WRN(TAG, MSG)
   Macro for sending a warning message to the currently registered
   MessageSink. The application specific tag may be any string, or an
   empty string. Messages with this level should be sparse, and
   indicate something that might be normal, but might result in
   unexpected behavior further on.

   @param TAG A string to associate the message with, or an empty
   string.

   @param MSG The message to send. This supports the output operator
   for easy printing of objects supporting this operator.
*/

/**\def GM_INF(TAG, MSG)
   Macro for sending an information message to the currently
   registered MessageSink. The application specific tag may be any
   string, or an empty string. Messages with this level should be
   sparse, and indicate something that might be necessary for the user
   to know, during normal operations.

   @param TAG A string to associate the message with, or an empty
   string.

   @param MSG The message to send. This supports the output operator
   for easy printing of objects supporting this operator.
*/

/**\def GM_DBG1(TAG, MSG)
   Macro for sending a debug message to the currently registered
   MessageSink. The application specific tag may be any string, or an
   empty string. Messages with this level report on high level
   internal processes. Do not send more than a few at a time and
   fairly seldom.

   @param TAG A string to associate the message with, or an empty
   string.

   @param MSG The message to send. This supports the output operator
   for easy printing of objects supporting this operator.
*/

/**\def GM_DBG2(TAG, MSG)
   Macro for sending a "verbose" debug message to the currently
   registered MessageSink. The application specific tag may be any
   string, or an empty string. Messages with this level may arrive at
   a higher rate, inside loops, but should not slow down the
   application.

   @param TAG A string to associate the message with, or an empty
   string.

   @param MSG The message to send. This supports the output operator
   for easy printing of objects supporting this operator.
*/

/**\def GM_DBG3(TAG, MSG)
   Macro for sending a "very verbose" debug message to the currently
   registered MessageSink. The application specific tag may be any
   string, or an empty string. Messages with this level may arrive at
   a high rate, inside loops, and may be so many that they can be
   expected to slow down the application.

   @param TAG A string to associate the message with, or an empty
   string.

   @param MSG The message to send. This supports the output operator
   for easy printing of objects supporting this operator.
*/

#ifdef NDEBUG

# define GM_ERR(TAG, MSG)                       \
  gramods::gmCore::Console                      \
  (gramods::gmCore::ConsoleLevel::Error,        \
   TAG) << MSG << std::endl

# define GM_WRN(TAG, MSG)                       \
  gramods::gmCore::Console                      \
  (gramods::gmCore::ConsoleLevel::Warning,      \
     TAG) << MSG << std::endl

# define GM_INF(TAG, MSG)                       \
  gramods::gmCore::Console                      \
  (gramods::gmCore::ConsoleLevel::Information,  \
   TAG) << MSG << std::endl

# define GM_DBG1(TAG, MSG)                      \
  gramods::gmCore::Console                      \
  (gramods::gmCore::ConsoleLevel::Debug1,       \
   TAG) << MSG << std::endl

# define GM_DBG2(TAG, MSG)                      \
  gramods::gmCore::Console                      \
  (gramods::gmCore::ConsoleLevel::Debug2,       \
   TAG) << MSG << std::endl

# define GM_DBG3(TAG, MSG)                      \
  gramods::gmCore::Console                      \
  (gramods::gmCore::ConsoleLevel::Debug3,       \
   TAG) << MSG << std::endl

#else // if NDEBUG else

/// TODO: replace with std::filesystem::path(__FILE__).filename() when
/// this is supported by mainstream GCC (version > 8.0)
#ifdef gramods_STRIP_PATH_FROM_FILE
#define GM_FILE gmCore::detail::strip_path(__FILE__)
#else
#define GM_FILE __FILE__
#endif

# define GM_ERR(TAG, MSG)                       \
  gramods::gmCore::Console                      \
  (gramods::gmCore::ConsoleLevel::Error,        \
   TAG, GM_FILE, __LINE__, __func__)            \
  << MSG << std::endl

# define GM_WRN(TAG, MSG)                       \
  gramods::gmCore::Console                      \
  (gramods::gmCore::ConsoleLevel::Warning,      \
   TAG, GM_FILE, __LINE__, __func__)            \
  << MSG << std::endl

# define GM_INF(TAG, MSG)                       \
  gramods::gmCore::Console                      \
  (gramods::gmCore::ConsoleLevel::Information,  \
   TAG, GM_FILE, __LINE__, __func__)            \
  << MSG << std::endl

# define GM_DBG1(TAG, MSG)                      \
  gramods::gmCore::Console                      \
  (gramods::gmCore::ConsoleLevel::Debug1,       \
   TAG, GM_FILE, __LINE__, __func__)            \
  << MSG << std::endl

# define GM_DBG2(TAG, MSG)                      \
  gramods::gmCore::Console                      \
  (gramods::gmCore::ConsoleLevel::Debug2,       \
   TAG, GM_FILE, __LINE__, __func__)            \
  << MSG << std::endl

# define GM_DBG3(TAG, MSG)                      \
  gramods::gmCore::Console                      \
  (gramods::gmCore::ConsoleLevel::Debug3,       \
   TAG, GM_FILE, __LINE__, __func__)            \
  << MSG << std::endl

#endif // if NDEBUG else endif

/**
   Console for easier handling of runtime and debugging
   information. For convenience, use the macros GM_ERR, GM_WRN,
   GM_INF, GM_DBG1, GM_DBG2 and GM_DBG3
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

  static void addSink(std::shared_ptr<MessageSink> ms) {
    std::lock_guard<std::mutex> guard(lock);
    message_sinks.push_back(ms);
  }

  static void removeSink(std::shared_ptr<MessageSink> ms) {
    std::lock_guard<std::mutex> guard(lock);
    message_sinks.erase(std::remove(message_sinks.begin(), message_sinks.end(), ms),
                        message_sinks.end());
  }

  static void removeAllSinks() {
    std::lock_guard<std::mutex> guard(lock);
    message_sinks.clear();
  }

  static MessageSink *getDefaultSink();

private:

  class ConsoleBuffer
      : public std::stringbuf {
    std::vector<std::shared_ptr<MessageSink>> sinks;
    MessageSink::Message message_template;
  public:
    ConsoleBuffer(std::vector<std::shared_ptr<MessageSink>> sinks,
                  MessageSink::Message msg)
      : sinks(sinks),
        message_template(msg) {}
    virtual int sync();
  };

  static ConsoleBuffer getBuffer(ConsoleLevel level, std::string tag);

  static ConsoleBuffer getBuffer(ConsoleLevel level, std::string tag,
                                 std::string file, int line, std::string function);

  static gmCore_API std::vector<std::shared_ptr<MessageSink>> message_sinks;

  ConsoleBuffer buffer;
  static gmCore_API std::mutex lock;
};

END_NAMESPACE_GMCORE;

#endif
