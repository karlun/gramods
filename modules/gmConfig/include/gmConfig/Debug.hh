/**
   Copyright 2003-2014, Karljohan Lundin Palmerius
   Copyright 2010, Loki Astari (http://stackoverflow.com)
   
*/


#ifndef _GRAMODS_DEBUG_HH_
#define _GRAMODS_DEBUG_HH_

#include <iostream>
#include <map>
#include <sstream>

#ifdef NDEBUG
# undef GRAMODS_DEBUG_LEVEL
# define GRAMODS_DEBUG_LEVEL 0
#endif

#ifndef GRAMODS_DEBUG_LEVEL
# define GRAMODS_DEBUG_LEVEL 2
#endif

namespace gramods {
  
  /**
     This is an ostream that prepends each line with a string to make it
     easier to see if there is an error or warning among information.
  */
  class DebugStream
    : public std::ostream {
    
    // Write a stream buffer that prefixes each line with Plop
    class DebugStreamBuf
      : public std::stringbuf {
      
      std::ostream &output;
      const std::string &prepend;
      
    public:
      
      DebugStreamBuf(const std::string &pre, std::ostream &str)
        : prepend(pre),
          output(str) {}
      
      virtual int sync() {
        output << prepend << str();
        str("");
        output.flush();
        return 0;
      }
    };
    
    // My Stream just uses a version of my special buffer
    DebugStreamBuf buffer;
    
  public:
    
    DebugStream(const std::string &pre, std::ostream &str)
      : std::ostream(&buffer),
      buffer(pre, str) {}
  };
}

#if GRAMODS_DEBUG_LEVEL > 1

# define GRAMODS_DEBUG_LOG_I(s)                                         \
  gramods::DebugStream("II ",std::cerr)                                 \
    << __FILE__ << ":" << __LINE__ << " (" << __FUNCTION__ << ")" << std::endl; \
  gramods::DebugStream("II   ",std::cerr)                               \
    << s << std::endl

# define GRAMODS_DEBUG_LOG()                                            \
  gramods::DebugStream("II ",std::cerr)                                 \
    << __FILE__ << ":" << __LINE__ << " (" << __FUNCTION__ << ")" << std::endl
  
#else
# define GRAMODS_DEBUG_LOG_I(s) // s
# define GRAMODS_DEBUG_LOG() // s
#endif
  
#if GRAMODS_DEBUG_LEVEL > 0
  
# define GRAMODS_DEBUG_LOG_W(s)                                         \
  gramods::DebugStream("WW ",std::cerr)                                 \
    << __FILE__ << ":" << __LINE__ << " (" << __FUNCTION__ << ")" << std::endl; \
  gramods::DebugStream("WW   ",std::cerr)                               \
    << s << std::endl

#else
# define GRAMODS_DEBUG_LOG_W(s) // s
#endif

# define GRAMODS_DEBUG_LOG_E(s)                                         \
  gramods::DebugStream("EE ",std::cerr)                                 \
    << __FILE__ << ":" << __LINE__ << " (" << __FUNCTION__ << ")" << std::endl; \
  gramods::DebugStream("EE   ",std::cerr)                               \
    << s << std::endl

# define GRAMODS_THROW(exception, X)                                    \
  throw (exception)(static_cast<std::stringstream&>(std::stringstream() << X).str())

#endif
