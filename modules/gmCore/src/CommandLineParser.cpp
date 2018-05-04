
#include <gmCore/CommandLineParser.hh>

BEGIN_NAMESPACE_GMCORE;

CommandLineParser::CommandLineParser(int &argc, char **&argv)
  : argc(argc), argv(argv), current_position(0) {}

bool CommandLineParser::getNextArgument(std::string &value) {
  if (current_position >= argc)
    return false;

  value = argv[current_position];
  ++current_position;

  return true;
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
