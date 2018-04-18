/**
   Copyright 2003-2018, Karljohan Lundin Palmerius
   Copyright 2010, Loki Astari (http://stackoverflow.com)
   
*/


#ifndef GRAMODS_CORE_CONSOLE
#define GRAMODS_CORE_CONSOLE

#include <gmCore/config.hh>

#include <gmCore/ConsoleLevel.hh>
#include <gmCore/MessageSink.hh>
#include <memory>
#include <sstream>

#include <iostream>
#include <string>

BEGIN_NAMESPACE_GMCORE;

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

#ifdef NDEBUG

# define GM_ERR(TAG, MSG)                                              \
  gramods::gmCore::Console(gramods::gmCore::ConsoleLevel::ERROR,           \
                         TAG) << MSG << std::endl
# define GM_WRN(TAG, MSG)                                              \
  gramods::gmCore::Console(gramods::gmCore::ConsoleLevel::WARNING,        \
                         TAG) << MSG << std::endl
# define GM_INF(TAG, MSG)                                            \
  gramods::gmCore::Console(gramods::gmCore::ConsoleLevel::INFORMATCOREN,  \
                         TAG) << MSG << std::endl
# define GM_VINF(TAG, MSG)                                              \
  gramods::gmCore::Console(gramods::gmCore::ConsoleLevel::VERBOSE_INFORMATCOREN, \
                         TAG) << MSG << std::endl
# define GM_VVINF(TAG, MSG)                                             \
  gramods::gmCore::Console(gramods::gmCore::ConsoleLevel::VERY_VERBOSE_INFORMATCOREN, \
                         TAG) << MSG << std::endl

#else // if NDEBUG else

#ifdef GM_STRIP_PATH_FROM_FILE
#define GM_FILE gmCore::detail::strip_path(__FILE__)
#else
#define GM_FILE __FILE__
#endif

# define GM_ERR(TAG, MSG)                                              \
  gramods::gmCore::Console(gramods::gmCore::ConsoleLevel::ERROR,           \
                         TAG, GM_FILE, __LINE__, __func__)            \
  << MSG << std::endl
# define GM_WRN(TAG, MSG)                                             \
  gramods::gmCore::Console(gramods::gmCore::ConsoleLevel::WARNING,        \
                         TAG, GM_FILE, __LINE__, __func__)           \
  << MSG << std::endl
# define GM_INF(TAG, MSG)                                            \
  gramods::gmCore::Console(gramods::gmCore::ConsoleLevel::INFORMATION,   \
                         TAG, GM_FILE, __LINE__, __func__)          \
  << MSG << std::endl
# define GM_VINF(TAG, MSG)                                              \
  gramods::gmCore::Console(gramods::gmCore::ConsoleLevel::VERBOSE_INFORMATION, \
                         TAG, GM_FILE, __LINE__, __func__)             \
  << MSG << std::endl
# define GM_VVINF(TAG, MSG)                                             \
  gramods::gmCore::Console(gramods::gmCore::ConsoleLevel::VERY_VERBOSE_INFORMATION, \
                         TAG, GM_FILE, __LINE__, __func__)             \
  << MSG << std::endl

#endif // if NDEBUG else endif

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

END_NAMESPACE_GMCORE;

#endif
