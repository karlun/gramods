
#ifndef GRAMODS_TEST_CONFIG
#define GRAMODS_TEST_CONFIG

#cmakedefine gramods_TEST_DEBUG_OUTPUT_TO_CONSOLE
#cmakedefine gramods_TEST_DEBUG_OUTPUT_TO_FILE

#include <gmCore/Console.hh>

#include <gmCore/LogFileMessageSink.hh>
#include <gmCore/NullMessageSink.hh>
#include <gmCore/OStreamMessageSink.hh>

namespace gramods::test {

void addConsoleMessageSinks() {

#ifdef gramods_TEST_DEBUG_OUTPUT_TO_CONSOLE
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->setUseAnsiColor(true);
  osms->setLevel(5);
  osms->initialize();
#endif

#ifdef gramods_TEST_DEBUG_OUTPUT_TO_FILE
  std::shared_ptr<gmCore::LogFileMessageSink> lfms =
      std::make_shared<gmCore::LogFileMessageSink>();
  lfms->setLogFilePath("gramods.log");
  lfms->setAppend(true);
  lfms->setLevel(5);
  lfms->initialize();
#endif
  
#if !defined(gramods_TEST_DEBUG_OUTPUT_TO_CONSOLE) &&                          \
    !defined(gramods_TEST_DEBUG_OUTPUT_TO_FILE)
  std::shared_ptr<gmCore::NullMessageSink> nullsink =
    std::make_shared<gmCore::NullMessageSink>();
  nullsink->initialize();
#endif
}

} // namespace gramods::tests

#endif
