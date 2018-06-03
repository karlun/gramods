
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

  /**
     Creates a new command line parser reading off arguments from the
     specified command line variables.
  */
  CommandLineParser(int &argc, char **&argv);

  /**
     Returns the next argument and increments the argument
     pointer. Throws exception if there are no more arguments.
  */
  std::string getNextArgument();

  /**
     Returns true if there are more arguments to extract, false otherwise.
  */
  bool hasMoreArguments();

  /**
     Removes the last arguments from the associnated command line
     variables.
  */
  bool consumeLast(int count);

private:

  int &argc;
  char **&argv;
  int current_position;
  int count;

};

END_NAMESPACE_GMCORE;

#endif
