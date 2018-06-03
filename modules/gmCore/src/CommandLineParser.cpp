
#include <gmCore/CommandLineParser.hh>

#include <stdexcept>

BEGIN_NAMESPACE_GMCORE;

CommandLineParser::CommandLineParser(int &argc, char **&argv)
  : argc(argc), argv(argv), current_position(0) {}

std::string CommandLineParser::getNextArgument() {
  if (current_position >= argc)
    throw std::out_of_range("No more arguments on command line");

  std::string value = argv[current_position];
  ++current_position;

  return value;
}

bool CommandLineParser::hasMoreArguments() {
  return current_position < argc;
}

bool CommandLineParser::consumeLast(int count) {
  if (current_position < count)
    return false;

  for (int idx = current_position - count; idx + count < argc; ++idx)
    argv[idx] = argv[idx + count];

  argc -= count;
  current_position -= count;

  return true;
}

END_NAMESPACE_GMCORE;
