
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/Stringify.hh>
#include <gmCore/InvalidArgument.hh>

#include <iostream>
#include <assert.h>

#ifdef _WIN32
// To enable setting output mode to handle virtual terminal sequences
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#  undef WIN32_LEAN_AND_MEAN
#endif

BEGIN_NAMESPACE_GMCORE;

GM_OFI_DEFINE_SUB(OStreamMessageSink, MessageSink);
GM_OFI_PARAM2(OStreamMessageSink, stream, std::string, setStream);
GM_OFI_PARAM2(OStreamMessageSink, useAnsiColor, bool, setUseAnsiColor);
GM_OFI_PARAM2(OStreamMessageSink, level, int, setLevel);

#define ANSI_DEBUG   "\033[90m"
#define ANSI_NORMAL  "\033[37m"
#define ANSI_ERROR   "\033[91m"
#define ANSI_WARNING "\033[93m"
#define ANSI_RESET   "\033[0m"


void OStreamMessageSink::output(Message msg) {
  if (msg.level > gramods::gmCore::ConsoleLevel(level)) return;
  std::lock_guard<std::mutex> guard(lock);

  std::ostream &out =
    shared_out ? *shared_out.get() :
    raw_out != nullptr ? *raw_out :
    (msg.level == ConsoleLevel::Error ||
     msg.level == ConsoleLevel::Warning) ?
    std::cerr : std::cout;

  if (use_ansi_color)
    switch (msg.level) {
    case ConsoleLevel::Error:
      out << ANSI_ERROR; break;
    case ConsoleLevel::Warning:
      out << ANSI_WARNING; break;
    case ConsoleLevel::Information:
      out << ANSI_NORMAL; break;
    case ConsoleLevel::Debug1:
    case ConsoleLevel::Debug2:
    case ConsoleLevel::Debug3:
      out << ANSI_DEBUG; break;
    }

#ifndef NDEBUG
  if (msg.source_data_available) {
    outputMetadata(out, msg);
    out << msg.file << ":" << msg.line << " (" << msg.function << ")\n";
  }
#endif

  std::istringstream input(msg.message);
  for (std::string line; std::getline(input, line);) {
    outputMetadata(out, msg);
    out << line << "\n";
  }

  if (use_ansi_color) out << ANSI_RESET;
  std::flush(out);
}

void OStreamMessageSink::setUseAnsiColor(bool on) {
#ifdef _WIN32

  if (!on) {
    use_ansi_color = false;
    return;
  }

  // Set output mode to handle virtual terminal sequences
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD dwMode = 0;
  if (hOut != INVALID_HANDLE_VALUE)
    if (GetConsoleMode(hOut, &dwMode)) {
      dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
      SetConsoleMode(hOut, dwMode);
    }

  hOut = GetStdHandle(STD_ERROR_HANDLE);
  dwMode = 0;
  if (hOut != INVALID_HANDLE_VALUE)
    if (GetConsoleMode(hOut, &dwMode)) {
      dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
      SetConsoleMode(hOut, dwMode);
    }

  use_ansi_color = true;

#else

  use_ansi_color = on;

#endif
}

void OStreamMessageSink::setStream(std::string name) {
  // no mutex lock since setStream engages the mutex
  if (name == "out") {
    setStream(&std::cout);
  } else if (name == "err") {
    setStream(&std::cerr);
  } else {
    throw gmCore::InvalidArgument(GM_STR("invalid stream name '" << name << "'"));
  }
}

END_NAMESPACE_GMCORE;
