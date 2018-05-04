
#ifndef GRAMODS_CORE_COMMANDLINEPARSER
#define GRAMODS_CORE_COMMANDLINEPARSER

#include <gmCore/config.hh>

#include <string>

BEGIN_NAMESPACE_GMCORE;

/**
   Simple parser for consuming command line arguments.

   For example, the command line "-abc file.xml --config file.xml" will be
   parsed as "-abc", "file.xml", "--config", "file.xml".
*/
class CommandLineParser {

public:

  CommandLineParser(int &argc, char **&argv);

  bool getNextArgument(std::string &value);

  bool hasMoreArguments();

  bool consumeLast(int count);

private:

  int &argc;
  char **&argv;
  int current_position;
  int count;

};

END_NAMESPACE_GMCORE;

#endif
